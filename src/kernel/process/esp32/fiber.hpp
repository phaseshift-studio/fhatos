#ifndef fhatos_kernel__fiber_hpp
#define fhatos_kernel__fiber_hpp

#include <fhatos.hpp>
//
#include <kernel/process/process.hpp>
#include <kernel/furi.hpp>

namespace fhatos::kernel {

class Fiber : public Process {

protected:
  bool _running = true;

public:
  TaskHandle_t handle{};
   explicit Fiber(const ID &id) : Process(id,FIBER) {}

   void delay(const uint64_t milliseconds) override {
    // delay to next fiber
     vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

   void yield() override {
    // do nothing
   }

   void stop() override { this->_running = false; }

    virtual const bool running() const override { return this->_running; }
};
} // namespace fhatos::kernel

#endif