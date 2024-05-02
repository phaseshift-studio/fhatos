#ifndef fhatos_kernel__mqtt_router_hpp
#define fhatos_kernel__mqtt_router_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/net/f_mqtt.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

template <class MESSAGE, class MQTT_CLIENT = fMQTT<Thread, MESSAGE>>
class MqttRouter : public Router<MESSAGE> {

public:
  inline static MqttRouter *singleton() {
    static MqttRouter singleton = MqttRouter();
    MQTT_CLIENT::singleton();
    return &singleton;
  }

  virtual RESPONSE_CODE clear() override {
    MQTT_CLIENT::singleton()->stop();
    return RESPONSE_CODE::OK;
  }

  MqttRouter(const ID id = fWIFI::idFromIP("mqttrouter"))
      : Router<MESSAGE>(id) {}

  virtual const RESPONSE_CODE publish(const MESSAGE &message) override {
    return MQTT_CLIENT::singleton()->publish(message.target, message.payload, message.retain)
               ? RESPONSE_CODE::OK
               : RESPONSE_CODE::ROUTER_ERROR;
  }
  virtual const RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) override {
    return MQTT_CLIENT::singleton()->subscribe(
               subscription.source, subscription.pattern,
               (uint8_t)subscription.qos,
               RecvFunction([subscription](const char *topic,
                                           const byte *payload,
                                           const int length) {
                 subscription.actor->push(Pair<Subscription<MESSAGE>, MESSAGE>(
                     subscription,
                     MESSAGE(topic,
                             subscription.source,
                             String((char*)payload,length),
                             true)));
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
    // return MQTT_CLIENT::singleton()->unsubscribeSource(source)
    //          ? RESPONSE_CODE::OK
    return RESPONSE_CODE::ROUTER_ERROR;
  }
};

} // namespace fhatos::kernel
#endif