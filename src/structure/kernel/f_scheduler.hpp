#ifndef fhatos_f_scheduler_hpp
#define fhatos_f_scheduler_hpp


#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include <structure/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fScheduler : public Actor<PROCESS, ROUTER> {
  public:
    static fScheduler *singleton() {
      static fScheduler scheduler = fScheduler();
      return &scheduler;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->onQuery(this->id().query("?"), [this](const SourceID&, const TargetID& target) {
        char temp[100];
        sprintf(temp, "\\_%s", target.query("").toString().c_str());
        this->publish(target, temp,RETAIN_MESSAGE);
      });
    }

    void loop() override {

    }

    bool spawn(Process *process) {
      return Scheduler::singleton()->spawn(process);
    }

  protected:
    fScheduler(const ID &id = fWIFI::idFromIP("kernel", "scheduler")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
