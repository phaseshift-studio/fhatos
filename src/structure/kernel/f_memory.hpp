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
      this->onQuery(this->id().query("?"), [this](const SourceID, const TargetID &target) {
        char temp[512];
        sprintf(temp,
                FOS_TAB "!b\\_!!!r%s!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                "[!gsketch:!b" FOS_BYTES_MB_STR "!!][!gheap!!:!b" FOS_BYTES_MB_STR "!!][!gpram!!:!b" FOS_BYTES_MB_STR
                "!!][!gflash!!:!b" FOS_BYTES_MB_STR "!!]\n",
                target.toString().c_str(),
                FOS_BYTES_MB(ESP.getFreeSketchSpace()),
                FOS_BYTES_MB(ESP.getFreeHeap()),
                FOS_BYTES_MB(ESP.getFreePsram()),
                FOS_BYTES_MB(ESP.getFlashChipSize()));
        this->publish(target, string(temp),RETAIN_MESSAGE);
      });
    }

  protected:
    fMemory(const ID &id = fWIFI::idFromIP("kernel", "memory")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
