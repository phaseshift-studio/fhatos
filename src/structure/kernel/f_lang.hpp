#ifndef fhatos_f_lang_hpp
#define fhatos_f_lang_hpp


#include <fhatos.hpp>
#include <language/parser.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include <structure/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Fiber, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fLang : public Actor<PROCESS, ROUTER> {
  public:
    static fLang *singleton() {
      static fLang lang = fLang();
      return &lang;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->subscribe(this->id().extend("parse"), [this](const Message &message) {
        Parser p = Parser(message.source);
        Bytecode *bcode = p.parse<Obj, Obj>(message.payload->toStr().toString().c_str());
        LOG_TASK(INFO, this, "%s\n", bcode->toString().c_str());
        this->publish(ID(message.source), BinaryObj<>::fromObj(new Str(bcode->toString())),RETAIN_MESSAGE);
      });
    }

  protected:
    fLang(const ID &id = fWIFI::idFromIP("kernel", "lang")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
