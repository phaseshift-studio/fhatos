#ifndef fhatos_kernel__threadx_hpp
#define fhatos_kernel__threadx_hpp

#include <fhatos.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Thread : public Process {

protected:
  bool __running = true;

public:
  TaskHandle_t handle;
  Thread(const ID &id) : Process(id) {}

  virtual void delay(const uint64_t milliseconds) override {
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

  virtual void yield() override { taskYIELD(); }

  virtual void setup() override {
    /*LOG(this->isEnabled() ? INFO : ERROR,
         "Scheduler starting %s thread %s (!rthreads:%i!!)\n",
         this->isLean() ? "lean" : "", this->id().c_str(),
         Scheduler->__THREADS.size());*/
    //if (this->running()) {
     // Scheduler::singleton()->addThread(this);
   // }
  }

  virtual void stop() override { this->__running = false; }

  virtual const bool running() const override { return this->__running; }
};
} // namespace fhatos::kernel

#endif