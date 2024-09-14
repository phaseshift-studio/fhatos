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
    ptr<connect_options> connection_options_;

    // +[scheme]//+[authority]/#[path]
    explicit Mqtt(const Pattern &pattern = Pattern("//+/#"),
                  string server_addr = STR(FOS_MQTT_BROKER_ADDR),
                  const Message_p &will_message = ptr<Message>(nullptr)) : BaseMqtt(
      pattern, server_addr, will_message) {
      this->server_addr_ = (string(server_addr).find_first_of("mqtt://") == string::npos
                              ? string("mqtt://").append(string(server_addr))
                              : server_addr);
      this->xmqtt_ = std::make_shared<async_client>(this->server_addr_, "", mqtt::create_options(MQTTVERSION_5));
      connect_options_builder pre_connection_options = connect_options_builder()
          .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
          .clean_start(true)
          .clean_session(true)
          .user_name(string("client_" + to_string(rand())))
          .keep_alive_interval(std::chrono::seconds(20))
          .automatic_reconnect();
      if (will_message.get()) {
        const BObj_p source_payload = will_message->payload->serialize();
        pre_connection_options = pre_connection_options.will(
          message(this->will_message_->target.toString(), source_payload->second, this->will_message_->retain));
      }
      this->connection_options_ = share(pre_connection_options.finalize());
      //// MQTT MESSAGE CALLBACK
      this->xmqtt_->set_message_callback([this](const const_message_ptr &mqtt_message) {
        const binary_ref ref = mqtt_message->get_payload_ref();
        const BObj_p bobj = share(BObj(ref.length(), reinterpret_cast<fbyte *>(const_cast<char *>(ref.data()))));
        const Message_p message = share(Message{
          .target = ID(mqtt_message->get_topic()),
          .payload = Obj::deserialize<Obj>(bobj),
          .retain = mqtt_message->is_retained()
        });
        LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
        const List_p<Subscription_p> matches = this->get_matching_subscriptions(furi_p(message->target));
        for (const Subscription_p &sub: *matches) {
          this->outbox_->push_back(share(Mail{sub, message}));
        }
        MESSAGE_INTERCEPT(message->target, message->payload, message->retain);
      });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      this->xmqtt_->set_connected_handler([this](const string &) {
        connection_logging(id_p(this->xmqtt_->get_client_id().c_str()));
      });
    }

    void native_mqtt_loop() override {
      this->loop();
    }


    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      this->xmqtt_->subscribe(subscription->pattern.toString(), static_cast<int>(subscription->qos))->wait();
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      this->xmqtt_->unsubscribe(pattern->toString())->wait();
    }

    void native_mqtt_publish(const Message_p &message) override {
      if (message->payload->is_noobj()) {
        this->xmqtt_->publish(message->target.toString().c_str(), const_cast<char *>(""), 0, 2, true)->wait();
      } else {
        const BObj_p source_payload = message->payload->serialize();
        this->xmqtt_->publish(
          string(message->target.toString().c_str()),
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
    static ptr<Mqtt> create(const Pattern &pattern, const char *server_addr = STR(FOS_MQTT_BROKER_ADDR),
                            const Message_p &will_message = ptr<Message>(nullptr)) {
      const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, server_addr, will_message));
      return mqtt_p;
    }

    // ~Mqtt() override {
    //   delete this->xmqtt_;
    // }

    void setup() override {
      Structure::setup();
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt_->connect(*this->connection_options_)->wait_for(1000)) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw mqtt::exception(1);
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s !yconnection!! retry\n", this->server_addr_.c_str());
            usleep(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if (this->xmqtt_->is_connected())
            break;
        }
      } catch (const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->server_addr_.c_str(), e.what());
      }
    }
  };
} // namespace fhatos
#endif
#endif
