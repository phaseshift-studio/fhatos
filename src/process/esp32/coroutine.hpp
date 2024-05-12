#ifndef fhatos_coroutine_hpp
#define fhatos_coroutine_hpp

#include <fhatos.hpp>
//
#include <furi.hpp>
#include <process/process.hpp>

namespace fhatos {
class Coroutine : public Process {
public:
  explicit Coroutine(const ID &id) : Process(id, COROUTINE) {}

  void delay(const uint64_t milliseconds) override {
    // do nothing
  }

  void yield() override {
    // do nothing
  }

  void loop() override {
    // do nothing
  }
};
} // namespace fhatos

#endif