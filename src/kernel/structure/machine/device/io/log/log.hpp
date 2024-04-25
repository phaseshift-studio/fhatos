#ifndef fhatos_kernel__log_hpp
#define fhatos_kernel__log_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/meta_router/meta_router.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/structure.hpp>
#include <kernel/util/ansi.hpp>
#include <kernel/util/string_stream.hpp>
#include <sstream>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

template <typename PROCESS = Thread, typename MESSAGE = StringMessage,
          typename ROUTER = LocalRouter<MESSAGE>>
class Log : public Actor<PROCESS, MESSAGE, ROUTER> {
public:
  Log(const ID &id = WIFI::idFromIP("log"))
      : Actor<PROCESS, MESSAGE, ROUTER>(id){};

  virtual void setup() override {
    const ID serialID = WIFI::idFromIP("serial");
    // INFO LOGGING
    this->subscribe(this->id().extend("INFO"), [this, serialID](
                                                   const MESSAGE &message) {
      this->publish(
          serialID,
          this->createLogMessage(INFO, message.payloadString()).c_str(), false);
    });
    // ERROR LOGGING
    this->subscribe(this->id().extend("ERROR"), [this, serialID](
                                                    const MESSAGE &message) {
      this->publish(
          serialID,
          this->createLogMessage(INFO, message.payloadString()).c_str(), false);
    });
  }

protected:
  const String createLogMessage(LOG_TYPE type, const String message) {
    if (message.startsWith("\t"))
      type = LOG_TYPE::NONE;
    String output;
    StringStream stream(&output);
    Ansi ansi = Ansi(&stream);
    if (type != LOG_TYPE::NONE)
      ansi.color(type == ERROR  ? ANSI::red
                 : type == INFO ? ANSI::green
                                : ANSI::yellow,
                 type == ERROR  ? "[ERROR] "
                 : type == INFO ? "[INFO]  "
                                : "[DEBUG] ");
    ansi.parse(message);
    ansi.flush();
    return output;
  }
};
} // namespace fhatos::kernel

#endif