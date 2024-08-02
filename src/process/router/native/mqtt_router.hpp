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
      static bool done = false;
      static MqttRouter mqtt = MqttRouter(id, serverAddr, willMessage);
      if (!done) {
        GLOBAL_OPTIONS->ROUTING = &mqtt;
        done = true;
      }
      return &mqtt;
    }

  protected:
    const char *serverAddr;
    async_client *xmqtt;
    List<Subscription_p> SUBSCRIPTIONS;
    MutexRW<> MUTEX_SUBSCRIPTIONS;
    Message_p willMessage;

    explicit MqttRouter(const ID &id = ID("/router/mqtt/"), const char *serverAddr = MQTT_BROKER_ADDR,
                        const Message_p &willMessage = ptr<Message>(nullptr)) :
        Router(id, ROUTER_LEVEL::GLOBAL_ROUTER), SUBSCRIPTIONS(List<Subscription_p>()),
        MUTEX_SUBSCRIPTIONS(MutexRW<>("mqtt-router")) {
      this->serverAddr = string(serverAddr).find_first_of("mqtt://") == string::npos
                             ? string("mqtt://").append(string(serverAddr)).c_str()
                             : serverAddr;
      this->xmqtt = new async_client(this->serverAddr, "", mqtt::create_options(MQTTVERSION_5));
      this->willMessage = willMessage;
      auto connection_options = connect_options_builder()
                                    .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
                                    .clean_start(true)
                                    .clean_session(true)
                                    .user_name("fhatos")
                                    .keep_alive_interval(std::chrono::seconds(20))
                                    .automatic_reconnect();
      if (willMessage.get())
        connection_options.will(
            message(this->willMessage->target.toString(), this->willMessage->payload.get(), this->willMessage->retain));
      //// MQTT MESSAGE CALLBACK
      this->xmqtt->set_message_callback([this](const const_message_ptr &mqttMessage) {
        const binary_ref ref = mqttMessage->get_payload_ref();
        const ptr<BObj> bobj = share(BObj(ref.length(), (fbyte *) ref.data()));
        const auto &[source, payload] = unwrapSource(bobj);
        LOG(TRACE, "Incoming MQTT message from %s to %s\n", source.toString().c_str(),
            mqttMessage->get_topic().c_str());
        const Message_p mess_ptr = share(Message{.source = source,
                                                 .target = ID(mqttMessage->get_topic()),
                                                 .payload = payload,
                                                 .retain = mqttMessage->is_retained()});
        auto _rc = MUTEX_SUBSCRIPTIONS.read<Pair<RESPONSE_CODE, List<Subscription_p>>>([this, mess_ptr] {
          //////////////
          RESPONSE_CODE _rc = mess_ptr->retain ? OK : NO_TARGETS;
          List<Subscription_p> subs;
          for (const auto &subscription: SUBSCRIPTIONS) {
            if (subscription->pattern.matches(mess_ptr->target)) {
              try {
                if (subscription->mailbox) {
                  _rc = subscription->mailbox->push(share<Mail>(Mail(subscription, mess_ptr))) ? OK : ROUTER_ERROR;
                  LOG(TRACE, "Message from !b%s!! delivered to mailbox !b%s!!: %s\n",
                      mess_ptr->source.toString().c_str(), subscription->source.toString().c_str(),
                      mess_ptr->payload->toString().c_str());
                  if (subscription->mailbox->size() > FOS_MAILBOX_WARNING_SIZE) {
                    LOG(WARN, "Mailbox !b%s!! reached warning size of %i: [size:%i]\n",
                        subscription->source.toString().c_str(), FOS_MAILBOX_WARNING_SIZE,
                        subscription->mailbox->size());
                  }
                } else {
                  subs.push_back(subscription);
                  _rc = OK;
                }
              } catch (const fError &e) {
                LOG_EXCEPTION(e);
                _rc = MUTEX_TIMEOUT;
              }
            }
          }
          LOG_PUBLISH(_rc, *mess_ptr);
          return std::make_pair(_rc, subs);
        });
        // process messages of non-threaded subscriptions (prevents mutex dead-lock as this devolves to chained method
        // calls)
        for (const auto &sub: _rc.second) {
          sub->onRecv(mess_ptr);
          LOG(TRACE, "Message from !b%s!! executing for !b%s!!\n", mess_ptr->source.toString().c_str(),
              sub->source.toString().c_str());
        }
        MESSAGE_INTERCEPT(mess_ptr->source, mess_ptr->target, mess_ptr->payload, mess_ptr->retain);
      });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      this->xmqtt->set_connected_handler([this](const string &) {
        LOG(INFO,
            "\n!g[!bMQTT Router Configuration!g]!!\n" FOS_TAB_2 "!bBroker address!!: %s\n" FOS_TAB_2
            "!bClient name!!   : %s\n" FOS_TAB_2 "!bWill topic!!    : %s\n" FOS_TAB_2
            "!bWill message!!  : %s\n" FOS_TAB_2 "!bWill QoS!!      : %i\n" FOS_TAB_2 "!bWill retain!!   : %s\n",
            this->serverAddr, this->xmqtt->get_client_id().c_str(),
            nullptr != this->willMessage.get() ? this->willMessage->target.toString().c_str() : "<none>",
            nullptr != this->willMessage.get() ? this->willMessage->payload->toString().c_str() : "<none>",
            GRANTED_QOS_1, nullptr != this->willMessage.get() ? FOS_BOOL_STR(this->willMessage->retain) : "<none>");
      });
      /// MQTT CONNECTION
      try {
        this->xmqtt->connect(connection_options.finalize());
        while (!this->xmqtt->is_connected()) {
          sleep(1);
          LOG(WARN, "Retrying connection to %s\n", this->serverAddr);
        }
      } catch (const mqtt::exception &e) {
        LOG(ERROR, "Unable to connect to remote server. Mqtt support not provided: %s\n", e.what());
      }
    }

  public:
    RESPONSE_CODE clear() override {
      /*for (const Subscription_p& sub: SUBSCRIPTIONS) {
        this->xmqtt->unsubscribe(sub->pattern.toString());
      }
      SUBSCRIPTIONS.clear();*/
      return OK;
    }

    RESPONSE_CODE publish(const Message &message) override {
      const ptr<BObj> bobj = wrapSource(message.source, message.payload);
      this->xmqtt->publish(message.target.toString(), bobj->second, bobj->first, 1, message.retain);
      const RESPONSE_CODE _rc = OK; //(RESPONSE_CODE) ret->get_return_code();
      LOG_PUBLISH(_rc, message);
      return _rc;
    }

    RESPONSE_CODE subscribe(const Subscription &subscription) override {
      const Subscription_p sub_ptr = share(subscription);
      try {
        /////////////// SUBSCRIPTION
        RESPONSE_CODE _rc = *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, sub_ptr]() {
          /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
          SUBSCRIPTIONS.erase(remove_if(SUBSCRIPTIONS.begin(), SUBSCRIPTIONS.end(),
                                        [sub_ptr](const Subscription_p &sub) {
                                          return sub->source.equals(sub_ptr->source) &&
                                                 sub->pattern.equals(sub_ptr->pattern);
                                        }),
                              SUBSCRIPTIONS.end());
          /////////////// ADD NEW SUBSCRIPTION
          bool found = false;
          for (Subscription_p s: SUBSCRIPTIONS) {
            if (s->pattern.equals(sub_ptr->pattern)) {
              found = true;
              break;
            }
          }
          RESPONSE_CODE _rc = OK;
          if (!found) {
            token_ptr token = this->xmqtt->subscribe(sub_ptr->pattern.toString(), static_cast<uint>(sub_ptr->qos));
            auto x = token->get_subscribe_response().get_reason_codes();
            LOG(DEBUG, "Subscription %s reason code: %i\n", sub_ptr->pattern.toString().c_str(), 1);
            _rc = OK;
          }
          if (!_rc) {
            SUBSCRIPTIONS.push_back(sub_ptr);
            LOG_SUBSCRIBE(_rc, sub_ptr);
          }
          return share<RESPONSE_CODE>(_rc);
        });
        return _rc;
      } catch (fError &e) {
        LOG_EXCEPTION(e);
        return ROUTER_ERROR;
      }
    }

    RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return unsubscribeX(source, &pattern);
    }

    RESPONSE_CODE unsubscribeSource(const ID &source) override { return unsubscribeX(source, nullptr); }

    void stop() override {
      this->clear();
      this->xmqtt->disconnect();
      LOG_TASK(INFO, this, "!b%s !ymqtt client to %s!! disconnected\n", this->id()->toString().c_str(),
               this->serverAddr);
    }

    RESPONSE_CODE unsubscribeX(const ID &source, const Pattern *pattern) {
      try {
        return *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, source, pattern]() {
          const uint16_t size = SUBSCRIPTIONS.size();
          SUBSCRIPTIONS.erase(remove_if(SUBSCRIPTIONS.begin(), SUBSCRIPTIONS.end(),
                                        [source, pattern](const auto &sub) {
                                          return sub->source.equals(source) &&
                                                 (nullptr == pattern || sub->pattern.equals(*pattern));
                                        }),
                              SUBSCRIPTIONS.end());
          const auto _rc2 =
              share<RESPONSE_CODE>(((SUBSCRIPTIONS.size() < size) || pattern == nullptr) ? OK : NO_SUBSCRIPTION);
          LOG_UNSUBSCRIBE(*_rc2, source, pattern);
          bool found = false;
          for (Subscription_p s: SUBSCRIPTIONS) {
            if (s->pattern.equals(*pattern)) {
              found = true;
              break;
            }
          }
          if (!found) {
            this->xmqtt->unsubscribe(pattern->toString());
          }
          return _rc2;
        });
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return MUTEX_TIMEOUT;
      }
    }
    const string toString() const override { return "MqttRouter"; }
  };
} // namespace fhatos
#endif
