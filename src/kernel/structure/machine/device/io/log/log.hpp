#ifndef fhatos_kernel__log_hpp
#define fhatos_kernel__log_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(fiber.hpp)

namespace fhatos::kernel {

template <typename MESSAGE> class Log : public Actor<Fiber, MESSAGE> {
public:
  Log(const ID &id = WIFI::idFromIP("log")) : Actor<Fiber,MESSAGE>(id){};
  virtual void setup() override {
    this->subscribe(this->id(), [this](MESSAGE message) {
      LOG(DEBUG, "%s\n", message.payloadString().c_str());
    });
  }
};
} // namespace fhatos::kernel

#endif