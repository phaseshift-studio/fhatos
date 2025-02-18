/*******************************************************************************
FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifdef ARDUINO
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "../mqtt.hpp"

//#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
//#endif

namespace fhatos {
  std::vector<Mqtt *> Mqtt::MQTT_VIRTUAL_CLIENTS = std::vector<Mqtt *>();
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////

  // TODO: mutiple pubsubs by broker w/ connection counter for connect/disconnect
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


  bool Mqtt::exists() const {
    return this->handler_.has_value() && std::any_cast<ptr<PubSubClient>>(this->handler_)->connected(); }

  Mqtt::Mqtt(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
    Structure(pattern, id_p(MQTT_FURI), value_id, config) {

    if(this->exists()) {
      LOG_STRUCTURE(INFO, this, "reusing existing connection to %s\n",
                    this->Obj::rec_get("config/broker")->toString().c_str());
      //MQTT_VIRTUAL_CLIENTS.push_back(this);
    } else if(this->Obj::rec_get("config/broker")->is_noobj()) {
      LOG_STRUCTURE(WARN, this, "mqtt disabled as no broker address provided\n");
    } else {
      MQTT_VIRTUAL_CLIENTS.push_back(this);
      //IPAddress host = IPAddress();
      //host.fromString(this->Obj::rec_get("config/broker")->uri_value().host());
      const char* host = strdup(this->Obj::rec_get("config/broker")->uri_value().host()); // TODO: get this off the heap
      const int port = this->Obj::rec_get("config/broker")->uri_value().port();
      WiFiClient* client = new WiFiClient(); //  TODO: get this off the heap
      const ptr<PubSubClient> h = std::make_shared<PubSubClient>(host, port, *client);
      this->handler_ = std::any(h);
      h->setServer(host, port);
      h->setBufferSize(MQTT_MAX_PACKET_SIZE);
      h->setSocketTimeout(100);
      h->setKeepAlive(100);
      h->setCallback([this](const char *topic, const uint8_t *data, const uint32_t length) {
        ((char *) data)[length] = '\0';
        const auto bobj = make_shared<BObj>(length, const_cast<fbyte *>(data));
        const auto [payload, retained] = make_payload(bobj);
        const Message_p message = Message::create(id_p(topic), payload, retained);
        LOG_STRUCTURE(DEBUG, this, "received message %s\n", message->toString().c_str());
        for(const auto *client: MQTT_VIRTUAL_CLIENTS) {
         //const auto matches = client->subscriptions_;// client->get_matching_subscriptions(*message->target());
          for(const Subscription_p &sub: *client->subscriptions_) {
            if(message->target()->bimatches(*sub->pattern()))
              client->outbox_->push_back(mail_p(sub, message));
          }
        }
      });
    }
  }

  void Mqtt::loop() {
    Structure::loop();
    const ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
    if(!h->connected()) {
      LOG_STRUCTURE(WARN, this, "reconnecting to mqtt broker: !r%s!!\n",
                    MQTT_STATE_CODES.at(h->state()).c_str());
      if(!h->connect(this->Obj::rec_get("config/client")->uri_value().toString().c_str())) {
        Process::current_process()->delay(FOS_MQTT_RETRY_WAIT);
      }
    }
    if(!h->loop()) {
      LOG_STRUCTURE(ERROR, this, "mqtt processing loop failure: !r%s!!\n",
                    MQTT_STATE_CODES.at(h->state()).c_str());
    }
  }

  void Mqtt::native_mqtt_subscribe(const Subscription_p &subscription) {
    const   ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
   h->subscribe(subscription->pattern()->toString().c_str(), 1);
    FEED_WATCHDOG();
    h->loop();
  }

  void Mqtt::native_mqtt_unsubscribe(const fURI &pattern) {
    const   ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
  h->unsubscribe(pattern.toString().c_str());
    FEED_WATCHDOG();
   h->loop();
  }

  void Mqtt::native_mqtt_publish(const Message_p &message) {
    const   ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
    if(message->payload()->is_noobj()) {
      h->publish(message->target()->toString().c_str(), nullptr, 0, message->retain());
    } else {
      const BObj_p source_payload = make_bobj(message->payload(), message->retain());
      h->publish(message->target()->toString().c_str(), source_payload->second, source_payload->first,
                               message->retain());
    }
    FEED_WATCHDOG();
  h->loop();
  }

  void Mqtt::native_mqtt_disconnect() {
    const   ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
    std::remove_if(MQTT_VIRTUAL_CLIENTS.begin(), MQTT_VIRTUAL_CLIENTS.end(),
                   [this](const Mqtt *m) { return m == this; });
    if(MQTT_VIRTUAL_CLIENTS.empty() && h->connected())
      h->disconnect();
  }

  void Mqtt::setup() {
    if(this->exists())
      return;
    try {
      Structure::setup();
      const ptr<PubSubClient> h =std::any_cast<ptr<PubSubClient>>(this->handler_);
      int counter = 0;
      while(counter < FOS_MQTT_MAX_RETRIES) {
        if(!h->connect(this->rec_get("config/client")->uri_value().toString().c_str())) {
          if(++counter > FOS_MQTT_MAX_RETRIES)
            throw fError("__wrapped below__");
          LOG_STRUCTURE(WARN, this, "!b%s !yconnection!! retry\n",
                        this->rec_get("config/broker")->uri_value().toString().c_str());
          Process::current_process()->delay(FOS_MQTT_RETRY_WAIT);
        }
        if(h->connected())
          break;
      }
      this->connection_logging();
    } catch(const fError &e) {
      LOG_STRUCTURE(ERROR, this, "unable to connect to !b%s!!: %s\n",
                    this->rec_get("config/broker")->uri_value().toString().c_str(), e.what());
    }
  }
}
#endif
