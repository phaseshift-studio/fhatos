#ifndef fhatos_fiber_hpp
#define fhatos_fiber_hpp

#include <fhatos.hpp>
//
#include <process/process.hpp>

namespace fhatos {
  class Fiber : public Process {
  public:
    TaskHandle_t handle{};

    explicit Fiber(const ID &id) : Process(id, FIBER) {
    }

    void delay(const uint64_t milliseconds) override {
      // delay to next fiber
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    }

    void yield() override {
      // do nothing
    }
  };
} // namespace fhatos

#endif
