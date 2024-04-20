#ifndef fhatos_kernel__task_hpp
#define fhatos_kernel__task_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Task : public IDed {

public:
  Task(const ID &id) : IDed(id) {}
  virtual void setup(){};
  virtual void start(){};
  virtual void stop(){};
  virtual bool running() { return true; }
  virtual void loop() {}
  virtual void delay(const uint64_t milliseconds){};
  virtual void yield(){};
};

} // namespace fhatos::kernel

#endif