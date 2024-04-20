#ifndef fhatos_kernel__fiber_hpp
#define fhatos_kernel__fiber_hpp

#include <fhatos.hpp>
#include <kernel/process/task/abstract_scheduler.hpp>
#include <kernel/process/task/esp32/scheduler.hpp>
#include <kernel/process/task/task.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Fiber : public Task {

protected:
  bool __running = true;

public:
  TaskHandle_t handle;
  Fiber(const ID &id) : Task(id) {}

  virtual void delay(const uint64_t milliseconds) override {
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

  virtual void yield() override { taskYIELD(); }

  virtual void setup() override {
    /*LOG(this->isEnabled() ? INFO : ERROR,
         "Scheduler starting %s thread %s (!rthreads:%i!!)\n",
         this->isLean() ? "lean" : "", this->id().c_str(),
         Scheduler->__THREADS.size());*/
    if (this->running()) {
      this->start();
    }
  }

  virtual void stop() override { this->__running = false; }

  virtual bool running() override { return this->__running; }
};
} // namespace fhatos::kernel

#endif