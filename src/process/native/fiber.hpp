#ifndef fhatos_fiber_hpp
#define fhatos_fiber_hpp

#include <fhatos.hpp>
//
#include FOS_PROCESS(process.hpp)

namespace fhatos {
  class Fiber : public Process {
  public:
    std::thread *xthread;

    explicit Fiber(const ID &id) : Process(id, FIBER) {
    }

    void delay(const uint64_t milliseconds) override {
      // delay to next fiber
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override {
      // do nothing
    }
  };
} // namespace fhatos

#endif
