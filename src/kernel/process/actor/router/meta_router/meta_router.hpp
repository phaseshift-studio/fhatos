#ifndef fhatos_kernel__meta_router_hpp
#define fhatos_kernel__meta_router_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/router/local_router/local_router.hpp>
#include <kernel/process/actor/router/mqtt_router/mqtt_router.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>

namespace fhatos::kernel {

template <class MESSAGE, typename LOCAL_ROUTER = LocalRouter<MESSAGE>,
          typename REMOTE_ROUTER = MqttRouter<MESSAGE>>
class MetaRouter : public Router<MESSAGE> {

protected:
  Router<MESSAGE> *select(const ID &target) {
    return false && this->id().isLocal(target)
               ? (Router<MESSAGE> *)LOCAL_ROUTER::singleton()
               : (Router<MESSAGE> *)REMOTE_ROUTER::singleton();
  }

public:
  inline static MetaRouter *singleton() {
    static MetaRouter singleton = MetaRouter();
    LOCAL_ROUTER::singleton();
    REMOTE_ROUTER::singleton();
    return &singleton;
  }

  MetaRouter(const ID &id = fWIFI::idFromIP("meta")) : Router<MESSAGE>(id) {}
  ~MetaRouter() { this->clear(); }
  virtual RESPONSE_CODE clear() override {
    RESPONSE_CODE __rc1 = LOCAL_ROUTER::singleton()->clear();
    RESPONSE_CODE __rc2 = REMOTE_ROUTER::singleton()->clear();
    return __rc1 == RESPONSE_CODE::OK ? __rc2 : __rc1;
  }

  virtual const RESPONSE_CODE publish(const MESSAGE &message) override {
    return this->select(message.target)->publish(message);
  }

  virtual const RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) override {
    return this->select(subscription.pattern)->subscribe(subscription);
  }

  virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                          const Pattern &pattern) override {
    return this->select(pattern)->unsubscribe(source, pattern);
  }

  virtual const RESPONSE_CODE unsubscribeSource(const ID &source) override {
    const RESPONSE_CODE local =
        LOCAL_ROUTER::singleton()->unsubscribeSource(source);
    const RESPONSE_CODE remote =
        REMOTE_ROUTER::singleton()->unsubscribeSource(source);
    return local ? local : remote;
  }
};
} // namespace fhatos::kernel

#endif
