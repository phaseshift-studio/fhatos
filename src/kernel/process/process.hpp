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
  virtual void start(){};
  virtual void stop(){};
  virtual bool running() { return true; }
  virtual void loop() {}
  virtual void delay(const uint64_t milliseconds){};
  virtual void yield(){};
};

class KernelProcess : public Process {
public:
  KernelProcess(const ID &id) : Process(id) {}
};

} // namespace fhatos::kernel

#endif