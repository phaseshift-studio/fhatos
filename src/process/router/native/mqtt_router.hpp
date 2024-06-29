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
#define MQTT_BROKER_ADDR "mqtt://localhost:1883"
#endif

using namespace mqtt;

namespace fhatos {
  class MqttRouter final : public Router {
  public:
    static MqttRouter *singleton() {
      static MqttRouter singleton = MqttRouter();
      singleton.setup();
      return &singleton;
    }

  protected:
    const char *server;
    uint16_t port;
    async_client *xmqtt;
    MutexDeque<Subscription_ptr> _SUBSCRIPTIONS;
    MutexDeque<Message_ptr> _PUBLICATIONS;
    string willTopic;
    string willMessage;
    bool willRetain{};
    uint8_t willQoS{};

    explicit MqttRouter(const ID &id = FOS_DEFAULT_ROUTER::mintID("kernel", "router/mqtt"),
                        const char *server = MQTT_BROKER_ADDR) : Router() {
      this->server = server;
      this->xmqtt = new async_client(this->server, "", mqtt::create_options(MQTTVERSION_5));
    }

  public:
    ~MqttRouter() = default;

    void setWill(const ID &willTopic, const string &willMessage, const bool willRetain = false,
                 const uint8_t willQoS = 1) {
      this->willTopic = willTopic.toString();
      this->willMessage = willMessage;
      this->willRetain = willRetain;
      this->willQoS = willQoS;
    }

    void setup() {
      uint8_t counter = 0;
      // message will = message(willTopic, willMessage, willQoS, willRetain);
      auto connection_options = connect_options_builder()
                                    .properties({{mqtt::property::SESSION_EXPIRY_INTERVAL, 604800}})
                                    .clean_start(false)
                                    // .will(std::move(will))
                                    .keep_alive_interval(std::chrono::seconds(20))
                                    .automatic_reconnect()
                                    .finalize();
      while (!this->xmqtt->is_connected() && (++counter < MQTT_MAX_RETRIES)) {
        // Attempt to connect
        try {
          this->xmqtt->connect(connection_options);
          if (this->xmqtt->is_connected()) {
            LOG(INFO, "!b[MQTT Router Configuration]!!\n");
            LOG(NONE,
                "\tBroker address          : %s:%i\n"
                "\tClient name             : %s\n"
                "\tWill topic              : %s\n"
                "\tWill message            : %s\n"
                "\tWill QoS                : %i\n"
                "\tWill retain             : %s\n",
                this->server, this->port, this->xmqtt->get_client_id().c_str(),
                this->willTopic.empty() ? "<none>" : this->willTopic.c_str(),
                this->willTopic.empty() ? "<none>" : this->willMessage.c_str(), this->willTopic.empty() ? -1 : willQoS,
                FOS_BOOL_STR(!this->willTopic.empty() && this->willRetain));

          } else {
            LOG(ERROR, "%s [retry in %ims]\n", this->server, MQTT_CONNECTION_RETRY);
            // Wait 5 seconds before retrying
            //   delay(MQTT_CONNECTION_RETRY);
          }
        } catch (const mqtt::exception &exc) {
          LOG(ERROR,
              "Unable to connect to remote server. Mqtt support not "
              "provided: %s.\n",
              exc.what());
        }
      }
      if (counter > MQTT_MAX_RETRIES) {
        // this->stop();
        LOG(ERROR, "Unable to connect to remote server. Mqtt support not "
                   "provided.\n");
      }
    }

    const RESPONSE_CODE clear() override {
      _SUBSCRIPTIONS.forEach([this](const Subscription_ptr &subscription) {
        this->xmqtt->unsubscribe(subscription->pattern.toString().c_str());
      });
      _SUBSCRIPTIONS.clear();
      _PUBLICATIONS.clear();
      return RESPONSE_CODE::OK;
    }

    const RESPONSE_CODE publish(const Message &message) override {
      const RESPONSE_CODE _rc =
          _PUBLICATIONS.push_back(share(message)) ? RESPONSE_CODE::OK : RESPONSE_CODE::ROUTER_ERROR;
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
          if (this->xmqtt->subscribe(subscription.pattern.toString().c_str(), (int) subscription.qos) &&
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

     void stop() {
      LOG(INFO, "Disconnecting MQTT from %s\n", this->server);
      _SUBSCRIPTIONS.forEach([this](const auto &sub) { this->unsubscribe(sub->source, sub->pattern); });
      _PUBLICATIONS.clear();
      _SUBSCRIPTIONS.clear();
      this->xmqtt->disconnect();
      //  PROCESS::stop();
    }

    void testConnection() {
      if (!this->xmqtt->is_connected()) {
        LOG(INFO, "Reconnecting to MQTT broker after connection loss\n");
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
        if (pattern &&
            !(_SUBSCRIPTIONS.find([pattern](const auto &sub) { return sub->pattern.equals(*pattern); }).has_value())) {
          _rc = this->xmqtt->unsubscribe(pattern->toString().c_str()) ? RESPONSE_CODE::OK : RESPONSE_CODE::ROUTER_ERROR;
            }
        return !_rc ? _rc : (_SUBSCRIPTIONS.size() < size ? RESPONSE_CODE::OK : RESPONSE_CODE::NO_SUBSCRIPTION);
      } catch (const std::runtime_error &e) {
        LOG_EXCEPTION(e);
        _rc = RESPONSE_CODE::MUTEX_TIMEOUT;
      };
      LOG_UNSUBSCRIBE(_rc, source, pattern);
      return _rc;
    }
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
