#ifndef fhatos_f_memory_hpp
#define fhatos_f_memory_hpp


#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include <structure/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Fiber, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fMemory : public Actor<PROCESS, ROUTER> {
  public:
    static fMemory *singleton() {
      static fMemory memory = fMemory();
      return &memory;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->onQuery(this->id(), [this](const SourceID, const TargetID &target) {
        char temp[100];
        sprintf(temp, "\\_%s", target.query("").toString().c_str());
        this->publish(target, temp,RETAIN_MESSAGE);
      });
    }

  protected:
    fMemory(const ID &id = fWIFI::idFromIP("kernel", "memory")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
