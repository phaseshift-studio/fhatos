#ifndef fhatos_f_kernel_hpp
#define fhatos_f_kernel_hpp


#include <fhatos.hpp>
#include FOS_MODULE(kernel/f_scheduler.hpp)
#include FOS_MODULE(kernel/f_memory.hpp)
#include FOS_MODULE(io/fs/f_fs.hpp)
#include FOS_MODULE(io/net/f_ota.hpp)
#include <process/actor/actor.hpp>
#include <structure/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fKernel : public Actor<PROCESS, ROUTER> {
  public:
    static const bool bootloader(const List<Process *>& processes) {
      Serial.begin(FOS_SERIAL_BAUDRATE);
      LOG(NONE, ANSI_ART);
      LOG(INFO, "!R[kernel mode]!! !gBootloader started!!\n");
      bool success = true;
      for (auto *process: processes) {
        success = success & Scheduler::singleton()->spawn(process);
      }
      LOG(INFO, "!R[kernel mode]!! !gBootloader finished!!\n");
      return success;
    }

    static fKernel *singleton() {
      static fKernel kernel = fKernel();
      return &kernel;
    }

    virtual void setup() override {
      Actor<PROCESS, ROUTER>::setup();
    }

  protected:
    fKernel(const ID &id = fWIFI::idFromIP("kernel")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
