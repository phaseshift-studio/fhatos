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

#include <structure/stype/mqtt/base_mqtt.hpp>
#include <fhatos.hpp>
#include <WiFiClient.h>
#include <PubSubClient.h>

namespace fhatos {

class Mqtt : public BaseMqtt {

  protected:
   ptr<PubSubClient> xmqtt_;

   explicit Mqtt(const Pattern &pattern = Pattern("//+/#"),
                 const char *server_addr = STR(FOS_MQTT_BROKER_ADDR),
                 const Message_p &will_message = ptr<Message>(nullptr)) : BaseMqtt(pattern, server_addr, will_message) {

  if (strcmp("none", server_addr) == 0) {
    LOG_STRUCTURE(WARN, this, "MQTT disabled as no broker address provided.\n");
  } else {
    WiFiClient *client = new WiFiClient();
    this->xmqtt_ = ptr<PubSubClient>(new PubSubClient(server_addr, FOS_MQTT_BROKER_PORT, *client));
    // this->xmqtt->setSocketTimeout(25);
    this->xmqtt_->setServer(server_addr, FOS_MQTT_BROKER_PORT);
    //this->xmqtt->setBufferSize(maxPacketSize);
    this->xmqtt_->setSocketTimeout(1000); // may be too excessive
    this->xmqtt_->setKeepAlive(1000);     // may be too excessive
    this->xmqtt_->setCallback([this](const char *topic, const uint8_t *data, const uint32_t length) {
        const BObj_p bobj = share(BObj(length, (fbyte *) data));
        const auto &[source, payload] = Message::unwrapSource(bobj);
        const Message_p message = share(Message{
          .source = *source,
          .target = ID(topic),
          .payload = payload,
          .retain = true //mqtt_message->is_retained()
        });
        LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
        const List_p<Subscription_p> matches = this->get_matching_subscriptions(furi_p(message->target));
        for (const Subscription_p &sub: *matches) {
          this->outbox_->push_back(share(Mail{sub, message}));
        }
        MESSAGE_INTERCEPT(message->source, message->target, message->payload, message->retain);
      });
  }
  }
  void native_mqtt_subscribe(const Subscription_p &subscription) override {
    this->xmqtt_->subscribe(subscription->pattern.toString().c_str(), static_cast<int>(subscription->qos));
  }

     void native_mqtt_unsubscribe(const fURI_p &pattern) override {
    this->xmqtt_->unsubscribe(pattern->toString().c_str());
  }

     void native_mqtt_publish(const Message_p &message) override {
    const BObj_p source_payload = Message::wrapSource(id_p(message->source), message->payload);
    this->xmqtt_->publish((const char*)message->target.toString().c_str(),(const uint8_t*)source_payload->second,(unsigned int)source_payload->first,(boolean)message->retain);
  }

     void native_mqtt_disconnect() override {
    this->xmqtt_->disconnect();
  }


  public:
  static ptr<Mqtt> create(const Pattern &pattern, const char *server_addr = STR(FOS_MQTT_BROKER_ADDR),
                       const Message_p &will_message = ptr<Message>(nullptr)) {
    const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, server_addr, will_message));
    return mqtt_p;
  }

    void setup() override {
      Structure::setup();
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt_->connect("abc")) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw fError("unable to connect");
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s:%i !yconnection!! retry\n", this->server_addr_, FOS_MQTT_BROKER_PORT);
            sleep(FOS_MQTT_RETRY_WAIT / 1000);
          }
          if (this->xmqtt_->connected())
            break;
        }
      } catch (const fError &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->server_addr_, e.what());
      }
    }

};

}

#endif
#endif