#ifndef fhatos_f_log_hpp
#define fhatos_f_log_hpp

#include <fhatos.hpp>
//
#include <structure/furi.hpp>
#include <process/actor/actor.hpp>
#include <process/router/local_router.hpp>
#include <structure/io/net/f_wifi.hpp>
#include <util/ansi.hpp>
#include <util/string_printer.hpp>
#include <sstream>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {

template <typename PROCESS = Coroutine, typename ROUTER = FOS_DEFAULT_ROUTER>
class fLog : public Actor<PROCESS, ROUTER> {
public:
  explicit fLog(const ID &id = fWIFI::idFromIP("log"))
      : Actor<PROCESS, ROUTER>(id) {};

  void setup() override {
    PROCESS::setup();
    const ID serialID = fWIFI::idFromIP("serial");
    // INFO LOGGING
    this->subscribe(
        this->id().extend("INFO"), [this, serialID](const auto &message) {
          this->publish(
              serialID,
              this->createLogMessage(INFO, message.payload->toString()),
              false);
        });
    // ERROR LOGGING
    this->subscribe(
        this->id().extend("ERROR"), [this, serialID](const auto &message) {
          this->publish(
              serialID,
              this->createLogMessage(INFO, message.payload->toString()),
              false);
        });
  }

protected:
  string createLogMessage(LOG_TYPE type, const string& message) {
    if (message[0] == '\t')
      type = LOG_TYPE::NONE;
    string output;
    StringPrinter stream = StringPrinter(&output);
    auto ansi = Ansi<StringPrinter>(&stream);
    if (type != LOG_TYPE::NONE) {
      if (type == ERROR)
        ansi.print("!r[ERROR]!!  ");
      else if (type == INFO)
        ansi.print("!g[INFO]!!  ");
      else
        ansi.print("!y[DEBUG]!!  ");
    }
    ansi.print(message.c_str());
    return string(output.c_str());
  }
};
} // namespace fhatos

#endif