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

#define EMC_TASK_STACK_SIZE 12000
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
#endif

#include "../mqtt_client.hpp"
#include <WiFi.h>
#include <espMqttClient.h>
#include "../../../../lang/mmadt/parser.hpp"

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 2500
#define FOS_MQTT_LOOP_TASK false

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
    const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
    if(this->is_connected()) {
      if(!FOS_MQTT_LOOP_TASK) {
        h->loop();
        // FEED_WATCHDOG();
      }
      while(true) {
        std::optional<Mail> m = this->next_mail();
        if(m.has_value()) {
          Mail *mp = &m.value();
          LOG_WRITE(INFO, this, L("message received: {}\n", mp->second->toString()));
          /*Memory::singleton()->use_custom_stack(InstBuilder::build("process_custom_stack")
                                                   ->inst_f([mp](const Obj_p &, const InstArgs &) {
                                                     mp->first->apply(mp->second);
                                                     return Obj::to_noobj();
                                                   })
                                                   ->create(),
                                               Obj::to_noobj(), 16000);*/
          // TODO: get this in a custom stack
          mp->first->apply(mp->second);
        } else
          break;
      }
    } else {
      this->connect(*this->source_);
    }
  }

  uint8_t *payloadbuffer = nullptr;
  size_t payloadbufferSize = 0;
  size_t payloadbufferIndex = 0;

  MqttClient::MqttClient(const Rec_p &config) :
      Rec(std::move(config->rec_value()), OType::REC, REC_FURI), Post(), handler_(nullptr), source_(nullptr) {
    this->handler_ = std::any(
        make_shared<espMqttClient>(FOS_MQTT_LOOP_TASK)); // bool internalTask = true, uint8_t priority = 1, uint8_t core = 1
    const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
    if(this->broker().host_is_ip()) {
      IPAddress ipaddr = IPAddress();
      ipaddr.fromString(this->broker().host());
      h->setServer(ipaddr, this->broker().port());
    } else
      h->setServer(this->broker().host(), this->broker().port());
    // char *client_id = strdup(this->client().toString().c_str());
    // h->setClientId(client_id);
    h->onDisconnect([this](espMqttClientTypes::DisconnectReason reason) {
      this->disconnect(*this->source_, false);
      this->connect(*this->source_);
    });
    h->onMessage([this](const espMqttClientTypes::MessageProperties &properties, const char *topic, const uint8_t *data,
                        size_t len, size_t index, size_t total) {
      if(total > MQTT_MAX_PACKET_SIZE)
        throw fError("mqtt message too large: %d > %d", total, MQTT_MAX_PACKET_SIZE);
      // start new packet, increase buffer size if necessary
      if(index == 0) {
        if(total > payloadbufferSize) {
          delete[] payloadbuffer;
          payloadbufferSize = total;
          payloadbuffer =
              static_cast<uint8_t *>(ps_malloc(payloadbufferSize)); // new(std::nothrow) uint8_t[payloadbufferSize];
          if(!payloadbuffer) {
            throw fError("mqtt message buffer could not be allocated [size:%d]", payloadbufferSize);
          }
        }
        payloadbufferIndex = 0;
      }
      // add data to buffer
      if(payloadbuffer) {
        memcpy(&payloadbuffer[payloadbufferIndex], data, len);
        payloadbufferIndex += len;
        if(payloadbufferIndex == total) {
          const auto bobj = make_shared<BObj>(total, (fbyte *) payloadbuffer);
          const auto [payload, retained] = make_payload(bobj);
          const Message_p message = Message::create(id_p(topic), payload, retained);
          //////////////////////////////////////////////////////////////////////////
          // optionally:
          // delete[] payloadbuffer;
          free(payloadbuffer);
          payloadbuffer = nullptr;
          payloadbufferSize = 0;
          //////////////////////////////////////////////////////////////////////////
          this->receive(message, false);
        }
      }
    });
  }

  void MqttClient::subscribe(const Subscription_p &subscription, const bool async) {
    this->subscriptions_->push_back(subscription);
    const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
    h->subscribe(subscription->pattern()->toString().c_str(), 0);
    h->loop();
  }

  void MqttClient::unsubscribe(const ID &source, const Pattern &pattern, const bool async) {
    this->subscriptions_->remove_if([this, &source, &pattern](const Subscription_p &sub) {
      const bool remove = pattern.bimatches(*sub->pattern()) && sub->source()->equals(source);
      if(remove) {
        const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
        h->unsubscribe(pattern.toString().c_str());
        FEED_WATCHDOG();
        h->loop();
      }
      return remove;
    });
  }

  void MqttClient::publish(const Message_p &message, const bool async) const {
    // const char* topic, uint8_t qos, bool retain, const char* payload
    const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
    if(message->payload()->is_noobj()) {
      h->publish(message->target()->toString().c_str(), 0, message->retain(), nullptr, 0);
    } else {
      const BObj_p source_payload = make_bobj(message->payload(), message->retain());
      h->publish(message->target()->toString().c_str(), 0, message->retain(), source_payload->second,
                 source_payload->first);
    }
    // FEED_WATCHDOG();
    h->loop();
  }

  bool MqttClient::disconnect(const ID &source, const bool async) {
    const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
    h->disconnect();
    this->clients_->remove(source);
    if(this->clients_->empty()) {
      CLIENTS.erase(this->broker());
    }
    h->clearQueue();
    LOG_WRITE(INFO, this, L("!ydisconnecting!! from !g[!y{}!g]!!\n", this->broker().toString()));
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool MqttClient::is_connected() const {
    return this->handler_.has_value() && std::any_cast<ptr<espMqttClient>>(this->handler_)->connected();
  }

  bool MqttClient::connect(const ID &source) {
    this->source_ = id_p(source);
    if(!this->clients_->exists(source))
      this->clients_->push_back(source);
    if(this->is_connected()) {
      LOG_WRITE(WARN, this, L("!b{} !yconnection!! already exists\n", this->broker().toString()));
      return true;
    }
    try {
      const auto h = std::any_cast<ptr<espMqttClient>>(this->handler_);
      int counter = 0;
      LOG_WRITE(
          INFO, this,
          L("!yattempting !b{} !ymqtt!! connection as !b{}!!", this->broker().toString(), this->client().toString()));
      h->disconnect();
      if(h->connect()) {
        int counter = 0;
        while(!this->is_connected()) {
          // h->loop();
          Thread::delay(750);
          Ansi<>::singleton()->print('.');
          Thread::yield();
          if(counter++ > 20) {
            Ansi<>::singleton()->print('\n');
            return false;
          }
        }
        Ansi<>::singleton()->print('\n');
        if(!this->subscriptions_->empty()) {
          LOG_WRITE(INFO, this,
                    L("!yresubscribing to subscription(s) !g[!msize:{}!g]!!\n", this->subscriptions_->size()));
          for(const auto &sub: *this->subscriptions_) {
            h->subscribe(sub->pattern()->toString().c_str(), 0);
          }
        }
      } else {
        return false;
      }
      // FEED_WATCHDOG();
      // this->on_connect();
    } catch(const std::exception &e) {
      return false;
    }
    return true;
  }
} // namespace fhatos
#endif
