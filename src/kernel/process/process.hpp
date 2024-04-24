#ifndef fhatos_kernel__process_hpp
#define fhatos_kernel__process_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Process : public IDed {

public:
  Process(const ID &id) : IDed(id) {}
  virtual void setup(){};
  virtual void loop() {}
  virtual void stop(){};
  virtual const bool running() const { return true; }
  virtual void delay(const uint64_t milliseconds){};
  virtual void yield(){};
};

class KernelProcess : public Process {
public:
  KernelProcess(const ID &id) : Process(id) {}
  virtual void stop() override { this->__running = false; };
  virtual const bool running() const override { return this->__running; }
  virtual void delay(const uint64_t milliseconds) override {
    ::delay(milliseconds);
  }
  virtual void yield() { ::yield(); }

protected:
  bool __running = true;
};

} // namespace fhatos::kernel

#endif