#ifndef fhatos_kernel__fiber_hpp
#define fhatos_kernel__fiber_hpp

#include <fhatos.hpp>
//
#include <kernel/process/process.hpp>
#include <kernel/furi.hpp>

namespace fhatos::kernel {

class Fiber : public Process {

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

};
} // namespace fhatos::kernel

#endif