#ifndef fhatos_kernel__log_hpp
#define fhatos_kernel__log_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/meta_router/meta_router.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

template <typename ROUTER = LocalRouter<StringMessage>>
class Log : public Actor<Thread, StringMessage, ROUTER> {
public:
  Log(const ID &id = WIFI::idFromIP("log"))
      : Actor<Thread, StringMessage, ROUTER>(id){};

  virtual void setup() override {
    this->subscribe(this->id().extend("INFO"), [this](const StringMessage& message) {
      LOG(INFO, message.payload.c_str());
    });
    this->subscribe(this->id().extend("ERROR"), [this](const StringMessage& message) {
      LOG(ERROR, message.payload.c_str());
    });
  }
};

class LogMessage : public Message<Pair<LOG_TYPE, String>> {

  LogMessage(const ID source, const ID target,
             const Pair<LOG_TYPE, String> payload, const bool retain)
      : Message<Pair<LOG_TYPE, String>>(source, target, payload, retain){};

  virtual const String payloadString() const override {
    return String("[") + payload.first + "]" + payload.second;
  }
};

} // namespace fhatos::kernel

#endif