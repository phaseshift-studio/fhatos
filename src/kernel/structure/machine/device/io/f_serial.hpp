#ifndef fhatos_kernel__f_serial_hpp
#define fhatos_kernel__f_serial_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/local_router.hpp>

namespace fhatos::kernel {

template <typename PROCESS = Fiber, typename PAYLOAD = String,
          typename ROUTER = LocalRouter<Message<PAYLOAD>>>
class fSerial : public Actor<PROCESS, PAYLOAD, ROUTER> {

protected:
  fSerial(const ID &id = fWIFI::idFromIP("serial"))
      : Actor<PROCESS, PAYLOAD, ROUTER>(id) {}

public:
  static void println(const char *text) {
    ROUTER::singleton()->publish(
        Message<PAYLOAD>{.source = "anonymous",
                        .target = fSerial::singleton()->id(),
                        .payload = String(text) + "\n",
                        .retain = false});
  }
  static fSerial *singleton() {
    static fSerial serial = fSerial();
    return &serial;
  }
  virtual void setup() override {
    this->subscribe(
        this->id(),
        [](const Message<PAYLOAD> &message) {
          Serial.print(message.payloadString().c_str());
        },
        QoS::_1);
  }
};
} // namespace fhatos::kernel

#endif