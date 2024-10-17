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

#ifndef NATIVE

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <fhatos.hpp>
#include <structure/stype/mqtt/base_mqtt.hpp>

#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
#endif

namespace fhatos {

  class Mqtt : public BaseMqtt {


  protected:
    const Map<int8_t, string> MQTT_STATE_CODES = {{{-4, "MQTT_CONNECTION_TIMEOUT"},
                                                   {-3, "MQTT_CONNECTION_LOST"},
                                                   {-2, "MQTT_CONNECT_FAILED"},
                                                   {-1, "MQTT_DISCONNECTED"},
                                                   {0, "MQTT_CONNECTED"},
                                                   {1, "MQTT_CONNECT_BAD_PROTOCOL"},
                                                   {2, "MQTT_CONNECT_BAD_CLIENT_ID"},
                                                   {3, "MQTT_CONNECT_UNAVAILABLE"},
                                                   {4, "MQTT_CONNECT_BAD_CREDENTIALS"},
                                                   {5, "MQTT_CONNECT_UNAUTHORIZED"}}};


    ptr<PubSubClient> xmqtt_;

    explicit Mqtt(const Pattern &pattern = Pattern("//+/#"), const Settings &settings = Settings()) :
        BaseMqtt(pattern, settings) {

      if (this->settings_.broker.empty()) {
        LOG_STRUCTURE(WARN, this, "mqtt disabled as no broker address provided\n");
      } else {
        WiFiClient *client = new WiFiClient();
        this->xmqtt_ = ptr<PubSubClient>(new PubSubClient(*client));
        this->xmqtt_->setServer(this->settings_.broker.c_str(), FOS_MQTT_BROKER_PORT);
        this->xmqtt_->setBufferSize(MQTT_MAX_PACKET_SIZE);
        this->xmqtt_->setSocketTimeout(1000); // may be too excessive
        this->xmqtt_->setKeepAlive(1000); // may be too excessive
        this->xmqtt_->setCallback([this](const char *topic, const uint8_t *data, const uint32_t length) {
          ((char *) data)[length] = '\0';
          // const fbyte* data_dup[length];
          // memcpy(data_dup,data,length);
          const BObj_p bobj = make_shared<BObj>(length, (fbyte *) data);
          const auto [payload, retained] = make_payload(bobj);
          // [payload,retain]
          const Message_p message = share(Message{.target = ID(topic), .payload = payload, .retain = retained});
          LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
          const List_p<Subscription_p> matches = this->get_matching_subscriptions(furi_p(topic));
          for (const Subscription_p &sub: *matches) {
            this->outbox_->push_back(share(Mail{sub, message}));
          }
          SCHEDULER_WRITE_INTERCEPT(message->target, message->payload, message->retain);
        });
      }
    }

    virtual void native_mqtt_loop() override {
      this->xmqtt_->loop();
      scheduler()->feed_local_watchdog();
    }

    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      this->xmqtt_->subscribe(subscription->pattern.toString().c_str(), 1);
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      this->xmqtt_->unsubscribe(pattern->toString().c_str());
    }

    void native_mqtt_publish(const Message_p &message) override {
      if (message->payload->is_noobj()) {
        this->xmqtt_->publish(message->target.toString().c_str(), nullptr, 0, true);
      } else {
        const Lst_p lst = Obj::to_lst({message->payload, dool(message->retain)});
        const BObj_p payload = make_bobj(message->payload, message->retain);
        this->xmqtt_->publish(message->target.toString().c_str(), payload->second, payload->first, message->retain);
      }
    }

    void native_mqtt_disconnect() override { this->xmqtt_->disconnect(); }


  public:
    static ptr<Mqtt> create(const Pattern &pattern = Pattern("//+/#"), const Settings &settings = Settings()) {
      static const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, settings));
      return mqtt_p;
    }

    void loop() override {
      Structure::loop();
      scheduler()->feed_local_watchdog();
      if (!this->xmqtt_->connected()) {
        LOG_STRUCTURE(INFO, this, "Reconnecting to MQTT broker after connection loss [%s]\n",
                      MQTT_STATE_CODES.at(this->xmqtt_->state()).c_str());
        if (!this->xmqtt_->connect("fhatos")) {
          Process::current_process()->delay(FOS_MQTT_RETRY_WAIT / 1000);
        }
      } else if (!this->xmqtt_->loop()) {
        LOG_STRUCTURE(ERROR, this, "MQTT processing loop failure: %s\n",
                      MQTT_STATE_CODES.at(this->xmqtt_->state()).c_str());
      }
    }

    void setup() override {
      Structure::setup();
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt_->connect("fhatos")) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw fError("__wrapped below__");
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s:%i !yconnection!! retry\n", this->settings_.broker.c_str(),
                          FOS_MQTT_BROKER_PORT);
            usleep(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if (this->xmqtt_->connected()) {
            connection_logging(id_p("fhatos"));
            break;
          }
        }
      } catch (const fError &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->settings_.broker.c_str(), e.what());
      }
    }
  };

} // namespace fhatos

#endif
#endif