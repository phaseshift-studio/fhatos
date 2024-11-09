//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once
#ifndef fhatos_mqtt_hpp
#define fhatos_mqtt_hpp

#ifdef NATIVE

#include <mqtt/async_client.h>
#include <structure/stype/mqtt/base_mqtt.hpp>
#include <unistd.h>

namespace fhatos {
  using namespace mqtt;
  class Mqtt;
  static ptr<async_client> MQTT_CONNECTION = nullptr;
  static List_p<Mqtt *> MQTT_VIRTUAL_CLIENTS = nullptr;

  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  class Mqtt : public BaseMqtt {
  protected:
    connect_options connection_options_{};

    bool exists() const override {
      return MQTT_CONNECTION && MQTT_CONNECTION->is_connected() &&
             MQTT_CONNECTION->get_server_uri() == this->settings_.broker_;
    }

    explicit Mqtt(const ID &id, const Pattern &pattern, const Settings &settings) :
      BaseMqtt(id, pattern, settings) {
      this->settings_.broker_ = string(string(ID(this->settings_.broker_).scheme()) == "mqtt"
                                         ? this->settings_.broker_
                                         : (string("mqtt://") + this->settings_.broker_));
      if (this->exists()) {
        LOG_STRUCTURE(INFO, this, "reusing existing connection to %s\n", this->settings_.broker_.c_str());
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      } else {
        MQTT_CONNECTION =
            std::make_shared<async_client>(this->settings_.broker_, this->settings_.client_, mqtt::create_options());
        connect_options_builder pre_connection_options = connect_options_builder()
            .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
            .clean_start(true)
            .clean_session(true)
            .user_name(this->settings_.client_)
            .keep_alive_interval(std::chrono::seconds(20))
            .automatic_reconnect();
        if (this->settings_.will_.get()) {
          const BObj_p source_payload = this->settings_.will_->payload->serialize();
          pre_connection_options = pre_connection_options.will(
              message(this->settings_.will_->target.toString(), source_payload->second, this->settings_.will_->retain));
        }
        this->connection_options_ = pre_connection_options.finalize();
        //// MQTT MESSAGE CALLBACK
        MQTT_CONNECTION->set_message_callback([this](const const_message_ptr &mqtt_message) {
          const binary_ref ref = mqtt_message->get_payload_ref();
          const auto bobj =
              std::make_shared<BObj>(ref.length(), reinterpret_cast<fbyte *>(const_cast<char *>(ref.data())));
          const auto [payload, retained] = make_payload(bobj);
          // assert(mqtt_message->is_retained() == retained); // TODO why does this sometimes not match?
          const Message_p message = message_p(ID(mqtt_message->get_topic()), payload, retained);
          LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
          for (const auto *client: *MQTT_VIRTUAL_CLIENTS) {
            const List_p<Subscription_p> matches = client->get_matching_subscriptions(furi_p(message->target));
            for (const Subscription_p &sub: *matches) {
              client->outbox_->push_back(share(Mail{sub, message}));
            }
          }
        });
        /// MQTT CONNECTION ESTABLISHED CALLBACK
        MQTT_CONNECTION->set_connected_handler([this](const string &) { connection_logging(); });
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      }
    }

    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      MQTT_CONNECTION->subscribe(subscription->pattern.toString(), 1)->wait();
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      MQTT_CONNECTION->unsubscribe(pattern->toString())->wait();
    }

    void native_mqtt_publish(const Message_p &message) override {
      if (message->payload->is_noobj()) {
        MQTT_CONNECTION->publish(message->target.toString().c_str(), const_cast<char *>(""), 0, 2, true)->wait();
      } else {
        const BObj_p source_payload = make_bobj(message->payload, message->retain);
        MQTT_CONNECTION
            ->publish(message->target.toString(), source_payload->second, source_payload->first,
                      1, // qos
                      message->retain)
            ->wait();
      }
    }

    void native_mqtt_disconnect() override {
      std::erase_if(*MQTT_VIRTUAL_CLIENTS, [this](const Mqtt *m) { return m == this; });
      if (MQTT_VIRTUAL_CLIENTS->empty())
        MQTT_CONNECTION->disconnect();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  public:
    static ptr<Mqtt> create(const ID& id, const Pattern &pattern, const Settings &settings) {
      if (!MQTT_VIRTUAL_CLIENTS)
        MQTT_VIRTUAL_CLIENTS = make_shared<List<Mqtt *>>();
      const auto mqtt_p = ptr<Mqtt>(new Mqtt(id, pattern, settings));
      return mqtt_p;
    }

    void setup() override {
      BaseMqtt::setup();
      if (this->exists())
        return;
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!MQTT_CONNECTION->connect(this->connection_options_)->wait_for(1000)) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw mqtt::exception(1);
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s !yconnection!! retry\n", this->settings_.broker_.c_str());
            usleep(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if (MQTT_CONNECTION->is_connected())
            break;
        }
      } catch (const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->settings_.broker_.c_str(), e.what());
      }
    }
  };
} // namespace fhatos
#endif
#endif