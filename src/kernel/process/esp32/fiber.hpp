#ifndef fhatos_kernel_fiber_hpp
#define fhatos_kernel_fiber_hpp

#include <fhatos.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Fiber : public Process {

protected:
  bool _running = true;

public:
  TaskHandle_t handle;
  explicit Fiber(const ID &id) : Process(id) {}

   void delay(const uint64_t milliseconds) override {
    // delay to next fiber
     vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

   void yield() override {
    // yield to next fiber
  }

   void setup() override {
    /*LOG(this->isEnabled() ? INFO : ERROR,
         "Scheduler starting %s thread %s (!rthreads:%i!!)\n",
         this->isLean() ? "lean" : "", this->id().c_str(),
         Scheduler->__THREADS.size());*/
  }

   void stop() override { this->_running = false; }

   const bool running() const override { return this->_running; }
};
} // namespace fhatos::kernel

#endif