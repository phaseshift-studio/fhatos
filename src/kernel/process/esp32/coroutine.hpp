#ifndef fhatos_kernel__coroutine_hpp
#define fhatos_kernel__coroutine_hpp

#include <fhatos.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {
class Coroutine : public Process {
public:
  Coroutine(const ID &id) : Process(id) {}
  virtual void setup(){};
  virtual void start() { __running = true; };
  virtual void stop() { __running = false; };
  virtual bool running() { return __running; }
  void loop() override {}
  virtual void delay(const uint64_t milliseconds){};
  virtual void yield(){};

protected:
  bool __running = true;
};
} // namespace fhatos::kernel

#endif