#ifndef fhatos_kernel__f_serial_hpp
#define fhatos_kernel__f_serial_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/router/local_router.hpp>

namespace fhatos::kernel {

template <typename PROCESS = Fiber, typename ROUTER = LocalRouter<>>
class fSerial : public Actor<PROCESS, ROUTER> {

protected:
 explicit fSerial(const ID &id = fWIFI::idFromIP("serial"))
      : Actor<PROCESS, ROUTER>(id) {}

public:
  static void println(const char *text) {
    ROUTER::singleton()->publish(
        Message{.source = "anonymous",
                .target = fSerial::singleton()->id(),
                .payload = {.type = STR,
                            .data = (byte *)(String(text) + "\n").c_str(),
                            .length = strlen(text) + 1},
                .retain = false});
  }
  static fSerial *singleton() {
    static fSerial serial = fSerial();
    return &serial;
  }
  virtual void setup() override {
    PROCESS::setup();
    this->subscribe(
        this->id(),
        [](const auto &message) {
          Serial.print(message.payload.toString().c_str());
        },
        QoS::_1);
  }
};
} // namespace fhatos::kernel

#endif