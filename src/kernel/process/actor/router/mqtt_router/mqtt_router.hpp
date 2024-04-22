#ifndef fhatos_kernel__mqtt_router_hpp
#define fhatos_kernel__mqtt_router_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

template <class MESSAGE, class MQTT_CLIENT = MQTT<Thread, MESSAGE>>
class MqttRouter : public Router<MESSAGE>, public Coroutine {

public:
  inline static MqttRouter *singleton() {
    static MqttRouter singleton = MqttRouter();
    return &singleton;
  }

  MqttRouter(const ID id = WIFI::idFromIP("mqttrouter"))
      : Router<MESSAGE>(id),Coroutine(id) {}

  virtual const RESPONSE_CODE publish(const MESSAGE &message) override {
    MQTT_CLIENT::singleton()->publish(message.target, message.payload);
    return RESPONSE_CODE::OK;
  }
  virtual const RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) override {
    return MQTT_CLIENT::singleton()->subscribe(
               subscription.source, subscription.pattern,
               (uint8_t)subscription.qos,
               RecvFunction([subscription](const char *topic,
                                                const byte *payload,
                                                const int length) {
                 subscription.actor->push(
                     {subscription,
                      MESSAGE{.source = "unknown",
                              .target = topic,
                              .payload = MESSAGE::fromBytes(payload, length),
                              .retain = true}});
               }))
               ? RESPONSE_CODE::OK
               : RESPONSE_CODE::REPEAT_SUBSCRIPTION;
  }
  virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                    const Pattern &pattern) override {
    return MQTT_CLIENT::singleton()->unsubscribe(source, pattern)
               ? RESPONSE_CODE::OK
               : RESPONSE_CODE::NO_SUBSCRIPTION;
  }
  virtual const RESPONSE_CODE unsubscribeSource(const ID &source) override {
    //return MQTT_CLIENT::singleton()->unsubscribeSource(source)
      //         ? RESPONSE_CODE::OK
            return RESPONSE_CODE::ROUTER_ERROR;
  }
};

} // namespace fhatos::kernel
#endif