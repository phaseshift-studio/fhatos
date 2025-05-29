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
#include "../mqtt_client.hpp"
#include <PubSubClient.h>
#include <WiFiClient.h>

// #ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
// #endif
/*
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
*/

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 2500

namespace fhatos {

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


  void MqttClient::loop() {
    const ptr<PubSubClient> h = std::any_cast<ptr<PubSubClient>>(this->handler_);
    if(!h->connected()) {
      LOG_WRITE(WARN, this, L("!yreconnecting to !b{}!!: !r{}!!\n", this->broker().toString(), MQTT_STATE_CODES.at(h->state())));
      while(!h->connect(this->client().toString().c_str())) {
        Thread::delay(FOS_MQTT_RETRY_WAIT);
        Thread::yield();
      }
      this->on_connect();
    }
    if(!h->loop()) {
      LOG_WRITE(ERROR, this, L("mqtt processing loop failure: !r{}!!\n", MQTT_STATE_CODES.at(h->state())));
    }
  }

  MqttClient::MqttClient(const Rec_p &config) :
      Rec(std::move(config->rec_value()), OType::REC, REC_FURI), handler_(nullptr) {
    //// MQTT MESSAGE CALLBACK]
    const char *host = strdup(this->broker().host()); // TODO: get this off the heap
    const int port = this->broker().port();
    WiFiClient *client = new WiFiClient(); //  TODO: get this off the heap
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
      this->on_recv(message);
    });
  }

  void MqttClient::subscribe(const Subscription_p &subscription, const bool async) const {
    this->subscriptions_->push_back(subscription);
    const auto h = std::any_cast<ptr<PubSubClient>>(this->handler_);
    h->subscribe(subscription->pattern()->toString().c_str(), 1);
    FEED_WATCHDOG();
    h->loop();
  }

  void MqttClient::unsubscribe(const ID &source, const fURI &pattern, const bool async) const {
    this->subscriptions_->remove_if([this, &source, &pattern](const Subscription_p &sub) {
      const bool remove = pattern.bimatches(*sub->pattern()) && sub->source()->equals(source);
      if(remove) {
        const ptr<PubSubClient> h = std::any_cast<ptr<PubSubClient>>(this->handler_);
        h->unsubscribe(pattern.toString().c_str());
        FEED_WATCHDOG();
        h->loop();
      }
      return remove;
    });
  }

  void MqttClient::publish(const Message_p &message, const bool async) const {
    const ptr<PubSubClient> h = std::any_cast<ptr<PubSubClient>>(this->handler_);
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

  bool MqttClient::disconnect(const ID &source, const bool async) const {
    this->unsubscribe(source, "#");
    this->clients_->remove(source);
    const ptr<PubSubClient> h = std::any_cast<ptr<PubSubClient>>(this->handler_);
    if(this->clients_->empty() && h->connected()) {
      h->disconnect();
      CLIENTS.erase(this->broker());
    }
    LOG_WRITE(INFO, this, L("!ydisconnecting!! from !g[!y{}!g]!!\n", this->broker().toString()));
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool MqttClient::is_connected() const {
    if(!this->handler_.has_value())
      return false;
    const auto h = std::any_cast<ptr<PubSubClient>>(this->handler_);
    bool conn = h->connected() /*&& h->get_server() == this->broker().toString()*/;
    return conn;
  }


  bool MqttClient::connect(const ID &source) const {
    if(this->is_connected()) {
      if(!this->clients_->exists(source))
        this->clients_->push_back(source);
      LOG_WRITE(WARN, this, L("!b{} !yconnection!! already exists\n", this->broker().toString()));
      return true;
    }
    try {
      const ptr<PubSubClient> h = std::any_cast<ptr<PubSubClient>>(this->handler_);
      int counter = 0;
      while(counter < FOS_MQTT_MAX_RETRIES) {
        if(!h->connect(this->client().toString().c_str())) {
          if(++counter > FOS_MQTT_MAX_RETRIES)
            throw fError("__wrapped below__");
          LOG_WRITE(WARN, this, L("!b{} !yconnection!! retry\n", this->broker().toString()));
          Thread::delay(FOS_MQTT_RETRY_WAIT);
        }
        if(h->connected()) {
          this->clients_->push_back(source);
          this->on_connect();
          return true;
        }
      }
    } catch(const std::exception &e) {
      LOG_WRITE(ERROR, this, L("unable to connect to !b{}!!: {}\n", this->broker().toString(), e.what()));
    }
    return false;
  }
} // namespace fhatos
#endif
