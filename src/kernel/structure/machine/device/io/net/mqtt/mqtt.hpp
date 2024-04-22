#ifndef fhatos_kernel__mqtt_hpp
#define fhatos_kernel__mqtt_hpp
#include <fhatos.hpp>
//
#include <PubSubClient.h>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include FOS_PROCESS(thread.hpp)

#define MQTT_MAX_RETRIES 10
#define MQTT_CONNECTION_RETRY 5000
#define MQTT_MAX_PACKET_SIZE 500
#define JSON_DOCUMENT_SIZE 250

#define RETAIN_MESSAGE true

namespace fhatos::kernel {

typedef std::function<void(const char *, const byte *, const uint32_t)>
    RecvFunction;

template <typename PROCESS, typename MESSAGE> class MQTT : public PROCESS {
public:
  static MQTT *singleton(const ID id = WIFI::idFromIP("mqtt"),
                         const char *domain = STR(MQTT_BROKER_ADDR),
                         const uint32_t port = MQTT_BROKER_PORT) {
    static MQTT singleton = MQTT(id, domain, port);
    return &singleton;
  }

  MQTT(const ID &id, const char *domain, const uint16_t port) : PROCESS(id) {
    if (strcmp("none", domain) == 0) {
      LOG(INFO, "MQTT disabled as no broker address provided.\n");
      this->stop();
    } else {
      WiFiClient *client = new WiFiClient();
      this->xmqtt = new PubSubClient(domain, port, *client);
      this->server = (char *)domain;
      this->port = port;
      // this->xmqtt->setSocketTimeout(25);
      this->xmqtt->setServer(domain, port);
      // this->xmqtt->setBufferSize(maxPacketSize);
      this->xmqtt->setSocketTimeout(1000); // may be too excessive
      this->xmqtt->setKeepAlive(1000);     // may be too excessive
      this->recvFunction = [this](const char *topic, const byte *payload,
                                  const int length) {
        ((char *)payload)[length] = '\0';
        LOG(INFO, "[!B%s!!] <=!mreceive!!= !B%s!!\n", topic, (char *)payload);
        for (const Subscription<MESSAGE> &sub : __SUBSCRIPTIONS) {
          if (ID(topic).matches(sub.pattern)) {
            sub.onRecv(MESSAGE{.source = ID("unknown"),
                               .target = topic,
                               .payload = String((char *)payload),
                               .retain = true});
          }
        }
      };
      this->xmqtt->setCallback(this->recvFunction);
    }
  }

  void onRecv(const RecvFunction recvFunction) {
    this->xmqtt->setCallback(recvFunction);
  };

  bool subscribe(const ID &source, const Pattern &topic, const uint8_t qos,
                 const OnRecvFunction<MESSAGE> onRecv) {
    bool found = false;
    if (!__SUBSCRIPTIONS.empty()) {
      for (const auto &sub : __SUBSCRIPTIONS) {
        if (sub.source.equals(source) && sub.topic.equals(topic)) {
          found = true;
          break;
        }
      }
    }
    bool success;
    if (found) {
      LOG_TASK(INFO, this, "Subscription from %s for %s already exists\n",
               source.toString().c_str(), topic.toString().c_str());
      success = false;
    } else {
      success = this->xmqtt->subscribe(topic.toString().c_str(), qos);
      LOG(success ? INFO : ERROR,
          "[!B%s!!] =!msubscribe[qos:%i]!!=> [!B%s!!]\n",
          source.toString().c_str(), qos, topic.toString().c_str());
      if (success) {
        this->__SUBSCRIPTIONS.push_front(Subscription<MESSAGE>{
            .source = source, .topic = topic, .qos = qos, .onRecv = onRecv});
      }
    }
    return success;
  }

  const bool unsubscribe(const ID &source, const Pattern &topic) {
    bool found = false;

    if (!__SUBSCRIPTIONS.empty()) {
      this->__SUBSCRIPTIONS.remove_if(
          [source, topic](const Subscription<MESSAGE> &sub) {
            return sub.source.equals(source) && sub.pattern.equals(topic);
          });
      for (const auto &sub : __SUBSCRIPTIONS) {
        if (sub.pattern.equals(topic)) {
          found = true;
          break;
        }
      }
    }
    bool success;
    if (found) {
      LOG(INFO, "[!B%s!!] =!munsubscribe!!=> [!B%s!!]\n",
          source.toString().c_str(), topic.toString().c_str());
      success = true;
    } else {
      success = this->xmqtt->unsubscribe(topic.toString().c_str());
      LOG(success ? INFO : ERROR, "[!B%s!!] =!munsubscribe!!=> [!B%s!!]\n",
          source.toString().c_str(), topic.toString().c_str());
    }
    return success;
  }

  void setup() override {
    if (!WIFI::singleton()->running() && !WIFI::singleton()->reconnect()) {
      this->stop();
      LOG_TASK(ERROR, this, "No WIFI connection. MQTT support not provided.\n");
    } else {
      uint8_t counter = 0;
      while (!this->xmqtt->connected() &&
             (MQTT_MAX_RETRIES == -1 || ++counter < MQTT_MAX_RETRIES)) {
        // Attempt to connect
        if ((this->willTopic.isEmpty() &&
             this->xmqtt->connect(
                 WIFI::singleton()->ip().toString().c_str())) ||
            (this->xmqtt->connect(WIFI::singleton()->ip().toString().c_str(),
                                  this->willTopic.c_str(), willQoS, willRetain,
                                  this->willMessage.c_str()))) {
          LOG(INFO, "!b[MQTT Client Configuration]!!\n");
          LOG(INFO,
              "\tID                      : %s\n"
              "\tBroker address          : %s:%i\n"
              "\tClient name             : %s\n"
              "\tWill topic              : %s\n"
              "\tWill message            : %s\n"
              "\tWill QoS                : %i\n"
              "\tWill retain             : %s\n",
              this->id().toString().c_str(), this->server, this->port,
              WIFI::singleton()->ip().toString().c_str(),
              this->willTopic.isEmpty() ? "<none>" : this->willTopic.c_str(),
              this->willTopic.isEmpty() ? "<none>" : this->willMessage.c_str(),
              this->willTopic.isEmpty() ? -1 : willQoS,
              FP_BOOL_STR(!this->willTopic.isEmpty() && this->willRetain));
          PROCESS::setup();
        } else {
          LOG(ERROR, "%s:%i [retry in %ims]\n", this->server, this->port,
              MQTT_CONNECTION_RETRY);
          // Wait 5 seconds before retrying
          delay(MQTT_CONNECTION_RETRY);
        }
        // yield();
      }
      if (!this->running() && counter > MQTT_MAX_RETRIES) {
        this->stop();
        LOG_TASK(ERROR, this,
                 "Unable to connect to remote server. Mqtt support not "
                 "provided.\n");
      }
    }
  }

  void loop() {

    this->testConnection();
    //  BEGIN: drain publication queue
    __DRAIN_PUBLICATION_QUEUE();
    //  END: drain publication queue
    // FP_ESP_FEED;
    if (!this->xmqtt->loop())
      LOG_TASK(ERROR, this, "MQTT processing loop failure\n");
  }

  bool publish(const char *target, const char *message, const bool retain) {
    this->__PUBLICATIONS.push_back(MESSAGE{.source = ID("unknown"),
                                           .target = ID(target),
                                           .payload = String(message),
                                           .retain = retain});
    return true;
  }

  void stop() {
    LOG_TASK(INFO, this, "Disconnecting MQTT from %s:%i\n", this->server,
             this->port);
    this->__DRAIN_PUBLICATION_QUEUE();
    this->xmqtt->loop();
    /*List<Subscription<MESSAGE>> __COPY =
    List<Subscription<MESSAGE>(__SUBSCRIPTIONS); for (const
    Subscription<MESSAGE> &sub : __COPY) { this->unsubscribe(sub.source,
    sub.pattern);
    };
    __COPY.clear();*/
    // FP_ESP_FEED;
    this->xmqtt->loop();
    this->xmqtt->disconnect();
    this->__SUBSCRIPTIONS.clear();
    PROCESS::stop();
  }
  void setWill(const ID &willTopic, const String &willMessage,
               const bool willRetain = false, const uint8_t willQoS = 1) {
    this->willTopic = willTopic;
    this->willMessage = willMessage;
    this->willRetain = willRetain;
    this->willQoS = willQoS;
  }
  bool unsubscribeSource(const ID id);
  bool running() { return this->__running || this->xmqtt->connected(); }
  const bool publish(const ID &topic, const String message,
                     const bool retain = false) {
    return this->publish(topic.toString().c_str(), message.c_str(), retain);
  }

private:
  List<MESSAGE> __PUBLICATIONS;
  List<Subscription<MESSAGE>> __SUBSCRIPTIONS;
  char *server;
  uint16_t port;
  PubSubClient *xmqtt;
  String willTopic;
  String willMessage;
  bool willRetain;
  uint8_t willQoS;
  RecvFunction recvFunction;
  void testConnection() {
    if (!this->running()) {
      LOG_TASK(INFO, this,
               "Reconnecting to MQTT broker after connection loss\n");
      this->setup();
    }
  }
  void __DRAIN_PUBLICATION_QUEUE() {
    for (const auto &it : __PUBLICATIONS) {
      // FP_ESP_FEED;
      if (this->xmqtt->publish(it.target.toString().c_str(),
                               it.payloadString().c_str(), it.retain)) {
        LOG(INFO, "!B%s!! =!mpublish[retain:%s]!!=> [!B%s!!] (!routbox:%i!!)\n",
            it.payloadString().c_str(), FP_BOOL_STR(it.retain),
            it.target.toString().c_str(), __PUBLICATIONS.size());
        __PUBLICATIONS.pop_front();
      } else {
        LOG(ERROR, "%s =!mpublish[retain:%s]!!=> [!B%s!!]\n",
            it.payloadString().c_str(), FP_BOOL_STR(it.retain),
            it.target.toString().c_str());
        this->xmqtt->clearWriteError();
        this->testConnection();
      }
    }
    if (!__PUBLICATIONS.empty())
      LOG(ERROR, "Not all MQTT publications published [remaining:%i]\n",
          __PUBLICATIONS.size());
    this->xmqtt->flush();
  };
  const Map<int8_t, String> MQTT_STATE_CODES = {
      {{-4, F("MQTT_CONNECTION_TIMEOUT")},
       {-3, F("MQTT_CONNECTION_LOST")},
       {-2, F("MQTT_CONNECT_FAILED")},
       {-1, F("MQTT_DISCONNECTED")},
       {0, F("MQTT_CONNECTED")},
       {1, F("MQTT_CONNECT_BAD_PROTOCOL")},
       {2, F("MQTT_CONNECT_BAD_CLIENT_ID")},
       {3, F("MQTT_CONNECT_UNAVAILABLE")},
       {4, F("MQTT_CONNECT_BAD_CREDENTIALS")},
       {5, F("MQTT_CONNECT_UNAUTHORIZED")}}};
};
} // namespace fhatos::kernel
#endif