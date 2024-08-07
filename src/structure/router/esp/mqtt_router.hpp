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
//
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <fhatos.hpp>
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/router/router.hpp>
#include <structure/io/net/f_wifi.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)

#define MQTT_MAX_RETRIES 10
#define MQTT_CONNECTION_RETRY 5000
#undef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 500
#define JSON_DOCUMENT_SIZE 250

#ifndef MQTT_BROKER_ADDR
#define MQTT_BROKER_ADDR "localhost:1883"
#endif

#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT 1883
#endif

namespace fhatos {
  class MqttRouter : public Router {
  protected:
    MqttRouter(const ID &id = Router::mintID("kernel", "router/mqtt"),
               const char *domain = STR(MQTT_BROKER_ADDR),
               const uint16_t port = MQTT_BROKER_PORT)
      : Router(id) {
      auto *client = new WiFiClient();
      this->xmqtt = new PubSubClient(domain, port, *client);
      this->server = (char *) domain;
      this->port = port;
      // this->xmqtt->setSocketTimeout(25);
      // this->xmqtt->setBufferSize(maxPacketSize);
      this->xmqtt->setServer(domain, port);
      this->xmqtt->setSocketTimeout(1000); // may be too excessive
      this->xmqtt->setKeepAlive(1000); // may be too excessive
      this->xmqtt->setCallback(
        [this](const char *target, const fbyte *payload, const int length) {
          ((char *) payload)[length] = '\0';
          const ID targetId = ID(target);
          _SUBSCRIPTIONS.forEach([targetId, payload,
              length](const Subscription &subscription) {
              if (targetId.matches(subscription.pattern)) {
                JsonDocument doc;
                deserializeJson(doc, payload, length);
                const Message message{
                  .source = ID(doc["source"].as<String>().c_str()),
                  .target = targetId,
                  .payload = Obj::deserialize<Obj>(
                  std::shared_ptr<
                  Pair<uint,unsigned char*>>(
                  new Pair<uint,unsigned char*>(
                  doc["length"].as<uint>(),
                  ( unsigned char*)doc["data"].as<const char *>()))),
                  .retain = doc["retain"].as<bool>()
                };
                LOG_RECEIVE(RESPONSE_CODE::OK, subscription, message);
                if (subscription.mailbox) {
                  subscription.mailbox->push(
                    share(Mail(share(subscription), share(message)))); // if mailbox, put in mailbox
                } else {
                  subscription.onRecv(share(message)); // else, evaluate callback
                }
                // delete[] results;
              }
            });
        });
    }

    char *server;
    uint16_t port;
    PubSubClient *xmqtt;
    WiFiClient *client;
    MutexDeque<Subscription> _SUBSCRIPTIONS;
    MutexDeque<Message> _PUBLICATIONS;
    String willTopic;
    String willMessage;
    bool willRetain{};
    uint8_t willQoS{};

  public:
    ~MqttRouter() {
      delete this->xmqtt;
      delete this->client;
      delete this->server;
    }

    static MqttRouter *singleton(const ID& id = "/router/mqtt/") {
      static MqttRouter singleton = MqttRouter(id);
      return &singleton;
    }

    void setWill(const ID &willTopic, const String &willMessage,
                 const bool willRetain = false, const uint8_t willQoS = 1) {
      this->willTopic = String(willTopic.toString().c_str());
      this->willMessage = willMessage;
      this->willRetain = willRetain;
      this->willQoS = willQoS;
    }

    virtual const RESPONSE_CODE clear() override {
      _SUBSCRIPTIONS.forEach([this](const Subscription &subscription) {
        this->xmqtt->unsubscribe(subscription.pattern.toString().c_str());
      });
      _SUBSCRIPTIONS.clear();
      _PUBLICATIONS.clear();
      return RESPONSE_CODE::OK;
    }

    virtual const RESPONSE_CODE publish(const Message &message) override {
      const RESPONSE_CODE _rc = _PUBLICATIONS.push_back(message)
                                  ? RESPONSE_CODE::OK
                                  : RESPONSE_CODE::ROUTER_ERROR;
      LOG_PUBLISH(_rc, message);
      return _rc;
    }

    virtual const RESPONSE_CODE
    subscribe(const Subscription &subscription) override {
      RESPONSE_CODE _rc =
          _SUBSCRIPTIONS
          .find([subscription](const auto &sub) {
            return subscription.source.equals(sub.source) &&
                   subscription.pattern.equals(sub.pattern);
          })
          .has_value()
            ? RESPONSE_CODE::REPEAT_SUBSCRIPTION
            : RESPONSE_CODE::OK;

      if (!_rc) {
        try {
          if (this->xmqtt->subscribe(subscription.pattern.toString().c_str()) &&
              _SUBSCRIPTIONS.push_back(subscription))
            _rc = RESPONSE_CODE::OK;
          else
            _rc = RESPONSE_CODE::ROUTER_ERROR;
        } catch (const std::runtime_error &e) {
          LOG_EXCEPTION(e);
          _rc = RESPONSE_CODE::MUTEX_TIMEOUT;
        }
      }
      LOG_SUBSCRIBE(_rc, &subscription);
      return _rc;
    }

    virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                            const Pattern &pattern) override {
      return unsubscribeX(source, &pattern);
    }

    virtual const RESPONSE_CODE unsubscribeSource(const ID &source) override {
      return unsubscribeX(source, nullptr);
    }

    virtual void setup() override {
      if (!fWIFI::singleton()->running() && !fWIFI::reconnect()) {
        this->stop();
        LOG_PROCESS(ERROR, this, "No WIFI connection. MQTT support not provided.\n");
      } else {
        uint8_t counter = 0;
        while (!this->xmqtt->connected() && (++counter < MQTT_MAX_RETRIES == -1 ||
                                             ++counter < MQTT_MAX_RETRIES)) {
          // Attempt to connect
          if ((this->willTopic.isEmpty() &&
               this->xmqtt->connect(fWIFI::ip().c_str())) ||
              (this->xmqtt->connect(fWIFI::ip().c_str(),
                                    this->willTopic.c_str(), willQoS, willRetain,
                                    this->willMessage.c_str()))) {
            LOG(INFO, "!b[MQTT Router Configuration]!!\n");
            LOG(NONE,
                "\tID                      : %s\n"
                "\tBroker address          : %s:%i\n"
                "\tClient name             : %s\n"
                "\tWill topic              : %s\n"
                "\tWill message            : %s\n"
                "\tWill QoS                : %i\n"
                "\tWill retain             : %s\n",
                this->id()->toString().c_str(), this->server, this->port,
                fWIFI::singleton()->ip().c_str(),
                this->willTopic.isEmpty() ? "<none>" : this->willTopic.c_str(),
                this->willTopic.isEmpty() ? "<none>" : this->willMessage.c_str(),
                this->willTopic.isEmpty() ? -1 : willQoS,
                FOS_BOOL_STR(!this->willTopic.isEmpty() && this->willRetain));
            Router::setup();
          } else {
            LOG(ERROR, "%s:%i [retry in %ims]\n", this->server, this->port,
                MQTT_CONNECTION_RETRY);
            // Wait 5 seconds before retrying
            delay(MQTT_CONNECTION_RETRY);
          }
        }
        if (counter > MQTT_MAX_RETRIES) {
          this->stop();
          LOG_PROCESS(ERROR, this,
                   "Unable to connect to remote server. Mqtt support not "
                   "provided.\n");
        }
      }
    }

    virtual void stop() override {
      LOG_PROCESS(INFO, this, "Disconnecting MQTT from %s:%i\n", this->server,
               this->port);
      _SUBSCRIPTIONS.forEach([this](const auto &sub) {
        this->unsubscribe(sub.source, sub.pattern);
      });
      _PUBLICATIONS.clear();
      _SUBSCRIPTIONS.clear();
      this->xmqtt->disconnect();
      Router::stop();
    }

    virtual void loop() override {
      uint8_t errors = 0;
      while (errors < 10) {
        const auto &m = _PUBLICATIONS.pop_front();
        if (m.has_value()) {
          JsonDocument doc;
          doc["source"] = m->source.toString();
          doc["type"] = (const uint) m->payload->o_type();
          doc["data"] = m->payload->serialize()->second;
          doc["length"] = m->payload->serialize()->first;
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
        LOG_PROCESS(ERROR, this, "Errors during publishing: %i\n", errors);
      }
      if (!this->xmqtt->loop()) {
        LOG_PROCESS(ERROR, this, "MQTT processing loop failure\n");
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
        LOG_PROCESS(INFO, this,
                 "Reconnecting to MQTT broker after connection loss\n");
        this->setup();
      }
    }

    const RESPONSE_CODE unsubscribeX(const ID &source, const Pattern *pattern) {
      RESPONSE_CODE _rc;
      try {
        const uint16_t size = _SUBSCRIPTIONS.size();
        /*while (true) {
          Option<Subscription<MESSAGE>> s = _SUBSCRIPTIONS.find(
              [source, pattern](const Subscription<MESSAGE> &sub) {
                return sub.source.equals(source) &&
                       (nullptr == pattern || sub.pattern.equals(*pattern));
              });
          if (!s.has_value())
            break;
          //else
            //_SUBSCRIPTIONS.remove(s.value());
        }*/

        /*_SUBSCRIPTIONS.remove_if(
             [source, pattern](const Subscription<MESSAGE> &sub) {
               return sub.source.equals(source) &&
                      (nullptr == pattern || sub.pattern.equals(*pattern));
             });*/
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
    }
  };
} // namespace fhatos
#endif
