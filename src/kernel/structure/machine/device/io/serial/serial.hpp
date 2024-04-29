#ifndef fhatos_kernel__serial_hpp
#define fhatos_kernel__serial_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/local_router/local_router.hpp>

namespace fhatos::kernel {

template <typename PROCESS = Thread, typename MESSAGE = String,
          typename ROUTER = LocalRouter<Message<MESSAGE>>>
class Serial : public Actor<PROCESS, MESSAGE, ROUTER> {

public:
  Serial(const ID &id = WIFI::idFromIP("serial"))
      : Actor<PROCESS, MESSAGE, ROUTER>(id) {}

  virtual void setup() override {
    Actor<PROCESS, MESSAGE, ROUTER>::setup();
    this->subscribe(
        this->id(),
        [](const Message<MESSAGE> &message) {
          ::Serial.print(message.payloadString().c_str());
        },
        QoS::_1);
  }
};
} // namespace fhatos::kernel

#endif