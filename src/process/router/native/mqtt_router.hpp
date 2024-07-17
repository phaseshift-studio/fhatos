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
#pragma once
#ifndef fhatos_mqtt_router_hpp
#define fhatos_mqtt_router_hpp

#include <fhatos.hpp>
#include <mqtt/async_client.h>
#include <process/router/router.hpp>

#define MQTT_MAX_RETRIES 10
#define MQTT_CONNECTION_RETRY 5000
#undef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 500
#define JSON_DOCUMENT_SIZE 250

#ifndef MQTT_BROKER_ADDR
#define MQTT_BROKER_ADDR "localhost:1883"
#endif

using namespace mqtt;

namespace fhatos {
  class MqttRouter final : public Router {
  public:
    static MqttRouter *singleton(const ID &id = ID("/router/mqtt/"), const char *serverAddr = MQTT_BROKER_ADDR,
                                 const Message_p &willMessage = ptr<Message>(nullptr)) {
      static MqttRouter mqtt = MqttRouter(id, serverAddr, willMessage);
      return &mqtt;
    }

  protected:
    const char *serverAddr;
    async_client *xmqtt;
    MutexDeque<Subscription_p> _SUBSCRIPTIONS;
    MutexDeque<Message_p> _PUBLICATIONS;
    Message_p willMessage;

    explicit MqttRouter(const ID &id = ID("/router/mqtt/"), const char *serverAddr = MQTT_BROKER_ADDR,
                        const Message_p &willMessage = ptr<Message>(nullptr)) :
        Router(id, ROUTER_LEVEL::GLOBAL_ROUTER) {
      this->serverAddr = string(serverAddr).find_first_of("mqtt://") == string::npos
                             ? string("mqtt://").append(string(serverAddr)).c_str()
                             : serverAddr;
      this->xmqtt = new async_client(this->serverAddr, "", mqtt::create_options(MQTTVERSION_5));
      this->willMessage = willMessage;
      auto connection_options = connect_options_builder()
                                    .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
                                    .clean_start(false)
                                    .keep_alive_interval(std::chrono::seconds(20))
                                    .automatic_reconnect();
      if (willMessage.get())
        connection_options.will(
            message(this->willMessage->target.toString(), this->willMessage->payload.get(), this->willMessage->retain));
      try {
        this->xmqtt->set_message_callback([this](const ptr<const message> &mqttMessage) {
          const ID mqttTopic = ID(mqttMessage->get_topic());
          _SUBSCRIPTIONS.forEach([mqttMessage, mqttTopic](const Subscription_p &subscription) {
            const binary_ref ref = mqttMessage->get_payload_ref();
            const ptr<BObj> bobj = share(BObj(ref.length(), (unsigned char *) ref.data()));
            if (mqttTopic.matches(subscription->pattern)) {
              const Message_p message = share(Message{.source = ID(FOS_DEFAULT_SOURCE_ID),
                                                      .target = mqttTopic,
                                                      .payload = Obj::deserialize<Obj>(bobj),
                                                      .retain = mqttMessage->is_retained()});
              LOG_RECEIVE(RESPONSE_CODE::OK, *subscription, *message);
              if (subscription->mailbox) {
                subscription->mailbox->push(share(Mail(subscription, message))); // if mailbox, put in mailbox
              } else {
                subscription->onRecv(message); // else, evaluate callback
              }
              // delete[] results;
            }
          });
        });
        this->xmqtt->set_connected_handler([this](const string &cause) {
          LOG(INFO,
              "\n!g[!bMQTT Router Configuration!g]!!\n" FOS_TAB_2 "!bBroker address!!: %s\n" FOS_TAB_2
              "!bClient name!!   : %s\n" FOS_TAB_2 "!bWill topic!!    : %s\n" FOS_TAB_2
              "!bWill message!!  : %s\n" FOS_TAB_2 "!bWill QoS!!      : %i\n" FOS_TAB_2 "!bWill retain!!   : %s\n",
              this->serverAddr, this->xmqtt->get_client_id().c_str(),
              nullptr != this->willMessage.get() ? this->willMessage->target.toString().c_str() : "<none>",
              nullptr != this->willMessage.get() ? this->willMessage->payload->toString().c_str() : "<none>",
              GRANTED_QOS_1, nullptr != this->willMessage.get() ? FOS_BOOL_STR(this->willMessage->retain) : "<none>");
        });
        this->xmqtt->connect(connection_options.finalize());
        while (!this->xmqtt->is_connected()) {
        }
      } catch (const mqtt::exception &e) {
        LOG(ERROR, "Unable to connect to remote server. Mqtt support not provided: %s\n", e.what());
      }
    }

  public:
    ~MqttRouter() override = default;

    const RESPONSE_CODE clear() override {
      _SUBSCRIPTIONS.forEach(
          [this](const Subscription_p &subscription) { this->xmqtt->unsubscribe(subscription->pattern.toString()); });
      _SUBSCRIPTIONS.clear();
      _PUBLICATIONS.clear();
      return RESPONSE_CODE::OK;
    }

    const RESPONSE_CODE publish(const Message &message) override {
      ptr<BObj> bobj = message.payload->serialize();
      delivery_token_ptr ret =
          this->xmqtt->publish(message.target.toString(), bobj->second, bobj->first, 1, message.retain);
      const RESPONSE_CODE _rc = OK; //(RESPONSE_CODE) ret->get_return_code();
      /*const RESPONSE_CODE _rc =
          _PUBLICATIONS.push_back(share(message)) ? RESPONSE_CODE::OK : RESPONSE_CODE::ROUTER_ERROR;*/
      LOG_PUBLISH(_rc, message);
      return _rc;
    }

    const RESPONSE_CODE subscribe(const Subscription &subscription) override {
      RESPONSE_CODE _rc =
          _SUBSCRIPTIONS
                  .find([subscription](const auto &sub) {
                    return subscription.source.equals(sub->source) && subscription.pattern.equals(sub->pattern);
                  })
                  .has_value()
              ? RESPONSE_CODE::REPEAT_SUBSCRIPTION
              : RESPONSE_CODE::OK;

      if (!_rc) {
        try {
          if (this->xmqtt->subscribe(subscription.pattern.toString(), (uint) subscription.qos) &&
              _SUBSCRIPTIONS.push_back(share(Subscription(subscription))))
            _rc = RESPONSE_CODE::OK;
          else
            _rc = RESPONSE_CODE::ROUTER_ERROR;
        } catch (const std::runtime_error &e) {
          LOG_EXCEPTION(e);
          _rc = RESPONSE_CODE::MUTEX_TIMEOUT;
        }
      }
      LOG_SUBSCRIBE(_rc, share(subscription));
      return _rc;
    }

    const RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return unsubscribeX(source, &pattern);
    }

    const RESPONSE_CODE unsubscribeSource(const ID &source) override { return unsubscribeX(source, nullptr); }

    void stop() override {
      _SUBSCRIPTIONS.forEach([this](const auto &sub) { this->unsubscribe(sub->source, sub->pattern); });
      _PUBLICATIONS.clear();
      _SUBSCRIPTIONS.clear();
      this->xmqtt->disconnect();
      LOG_TASK(INFO, this, "!b%s !ymqtt client to %s!! disconnected\n", this->id()->toString().c_str(),
               this->serverAddr);
    }

    const RESPONSE_CODE unsubscribeX(const ID &source, const Pattern *pattern) {
      RESPONSE_CODE _rc = RESPONSE_CODE::OK;
      try {
        const uint16_t size = _SUBSCRIPTIONS.size();
        if (pattern &&
            !(_SUBSCRIPTIONS.find([pattern](const auto &sub) { return sub->pattern.equals(*pattern); }).has_value())) {
          _rc = this->xmqtt->unsubscribe(pattern->toString()) ? RESPONSE_CODE::OK : RESPONSE_CODE::ROUTER_ERROR;
        }
        return !_rc ? _rc : (_SUBSCRIPTIONS.size() < size ? RESPONSE_CODE::OK : RESPONSE_CODE::NO_SUBSCRIPTION);
      } catch (const std::runtime_error &e) {
        LOG_EXCEPTION(e);
        _rc = RESPONSE_CODE::MUTEX_TIMEOUT;
      };
      LOG_UNSUBSCRIBE(_rc, source, pattern);
      return _rc;
    }

    const string toString() const override { return "MqttRouter"; }
  };
} // namespace fhatos


/*
   virtual void loop() override {
     uint8_t errors = 0;
     while (errors < 10) {
       const auto &m = _PUBLICATIONS.pop_front();
       if (m.has_value()) {
         JsonDocument doc;
         doc["source"] = m->source.toString();
         doc["type"] = (const uint) m->payload->type;
         doc["data"] = m->payload->data;
         doc["length"] = m->payload->length;
         doc["retain"] = m->retain;
         char buffer[512];
         const uint length = serializeJson(doc, buffer);
         if (!this->xmqtt->publish(m->target.toString().c_str(),
                                   (const fbyte *) buffer, length, m->retain)) {
           LOG(ERROR, "%s=!mpublish[retain:%s]!!=> [!B%s!!] (!R%s!!)\n", buffer,
               FOS_BOOL_STR(m->retain), m->target.toString().c_str(),
               MQTT_STATE_CODES.at(this->xmqtt->getWriteError()).c_str());
           this->xmqtt->clearWriteError();
           errors++;
         }
       } else {
         // delete m->payload.data;
         break;
       }
     }
     if (errors) {
       LOG_TASK(ERROR, this, "Errors during publishing: %i\n", errors);
     }
     if (!this->xmqtt->loop()) {
       LOG_TASK(ERROR, this, "MQTT processing loop failure\n");
       this->testConnection();
     }
   }

 private:
   const Map<int8_t, String> MQTT_STATE_CODES = {
     {
       {-4, F("MQTT_CONNECTION_TIMEOUT")},
       {-3, F("MQTT_CONNECTION_LOST")},
       {-2, F("MQTT_CONNECT_FAILED")},
       {-1, F("MQTT_DISCONNECTED")},
       {0, F("MQTT_CONNECTED")},
       {1, F("MQTT_CONNECT_BAD_PROTOCOL")},
       {2, F("MQTT_CONNECT_BAD_CLIENT_ID")},
       {3, F("MQTT_CONNECT_UNAVAILABLE")},
       {4, F("MQTT_CONNECT_BAD_CREDENTIALS")},
       {5, F("MQTT_CONNECT_UNAUTHORIZED")}
     }
   };

   void testConnection() {
     if (!this->running()) {
       LOG_TASK(INFO, this,
                "Reconnecting to MQTT broker after connection loss\n");
       this->setup();
     }
   }

   const RESPONSE_CODE unsubscribeX(const ID &source, const Pattern *pattern) {
     RESPONSE_CODE _rc;
     try {
       const uint16_t size = _SUBSCRIPTIONS.size();
       while (true) {
         Option<Subscription<MESSAGE>> s = _SUBSCRIPTIONS.find(
             [source, pattern](const Subscription<MESSAGE> &sub) {
               return sub.source.equals(source) &&
                      (nullptr == pattern || sub.pattern.equals(*pattern));
             });
         if (!s.has_value())
           break;
         //else
           //_SUBSCRIPTIONS.remove(s.value());
       }

       /SUBSCRIPTIONS.remove_if(
            [source, pattern](const Subscription<MESSAGE> &sub) {
              return sub.source.equals(source) &&
                     (nullptr == pattern || sub.pattern.equals(*pattern));
            });
       if (pattern && !(_SUBSCRIPTIONS
             .find([pattern](const auto &sub) {
               return sub.pattern.equals(*pattern);
             })
             .has_value())) {
         _rc = this->xmqtt->unsubscribe(pattern->toString().c_str())
                 ? RESPONSE_CODE::OK
                 : RESPONSE_CODE::ROUTER_ERROR;
       }
       return !_rc
                ? _rc
                : (_SUBSCRIPTIONS.size() < size
                     ? RESPONSE_CODE::OK
                     : RESPONSE_CODE::NO_SUBSCRIPTION);
     } catch (const std::runtime_error &e) {
       LOG_EXCEPTION(e);
       _rc = RESPONSE_CODE::MUTEX_TIMEOUT;
     };
     LOG_UNSUBSCRIBE(_rc, source, pattern);
     return _rc;
   }*/
#endif
