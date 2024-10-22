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

#include <structure/stype/mqtt/base_mqtt.hpp>
#include <mqtt/async_client.h>
#include <unistd.h>

namespace fhatos {
  using namespace mqtt;

  class Mqtt : public BaseMqtt {
  protected:
    ptr<async_client> xmqtt_;
    connect_options connection_options_{};

    explicit Mqtt(const Pattern &pattern = Pattern("//+/#"), const Settings &settings = Settings()) : BaseMqtt(
      pattern, settings) {
      this->settings_.broker = string(this->settings_.broker).find_first_of("mqtt://") == string::npos
                                 ? string("mqtt://").append(string(this->settings_.broker))
                                 : settings_.broker;
      this->xmqtt_ = std::make_shared<async_client>(this->settings_.broker, this->settings_.client,
                                                    mqtt::create_options());
      connect_options_builder pre_connection_options = connect_options_builder()
          .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
          .clean_start(true)
          .clean_session(true)
          .user_name(this->settings_.client)
          .keep_alive_interval(std::chrono::seconds(20))
          .automatic_reconnect();
      if (this->settings_.will.get()) {
        const BObj_p source_payload = this->settings_.will->payload->serialize();
        pre_connection_options = pre_connection_options.will(
          message(this->settings_.will->target.toString(), source_payload->second, this->settings_.will->retain));
      }
      this->connection_options_ = pre_connection_options.finalize();
      //// MQTT MESSAGE CALLBACK
      this->xmqtt_->set_message_callback([this](const const_message_ptr &mqtt_message) {
        const binary_ref ref = mqtt_message->get_payload_ref();
        const auto bobj = std::make_shared<BObj>(ref.length(),
                                                 reinterpret_cast<fbyte *>(const_cast<char *>(ref.data())));
        const auto [payload, retained] = make_payload(bobj);
        // assert(mqtt_message->is_retained() == retained); // TODO why does this sometimes not match?
        const Message_p message = message_p(ID(mqtt_message->get_topic()), payload, retained);
        LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
        const List_p<Subscription_p> matches = this->get_matching_subscriptions(furi_p(message->target));
        for (const Subscription_p &sub: *matches) {
          this->outbox_->push_back(share(Mail{sub, message}));
        }
      });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      this->xmqtt_->set_connected_handler([this](const string &) {
        connection_logging();
      });
    }

    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      this->xmqtt_->subscribe(subscription->pattern.toString(), 1)->wait();
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      this->xmqtt_->unsubscribe(pattern->toString())->wait();
    }

    void native_mqtt_publish(const Message_p &message) override {
      if (message->payload->is_noobj()) {
        this->xmqtt_->publish(message->target.toString().c_str(), const_cast<char *>(""), 0, 2, true)->wait();
      } else {
        const BObj_p source_payload = make_bobj(message->payload, message->retain);
        this->xmqtt_->publish(
          message->target.toString(),
          source_payload->second,
          source_payload->first,
          1, //qos
          message->retain)->wait();
      }
    }

    void native_mqtt_disconnect() override {
      this->xmqtt_->disconnect();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  public:
    static ptr<Mqtt> create(const Pattern &pattern, const Settings &settings = Settings()) {
      const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, settings));
      return mqtt_p;
    }

    void setup() override {
      BaseMqtt::setup();
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt_->connect(this->connection_options_)->wait_for(1000)) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw mqtt::exception(1);
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s !yconnection!! retry\n", this->settings_.broker.c_str());
            usleep(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if (this->xmqtt_->is_connected())
            break;
        }
      } catch (const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->settings_.broker.c_str(), e.what());
      }
    }
  };
} // namespace fhatos
#endif
#endif
