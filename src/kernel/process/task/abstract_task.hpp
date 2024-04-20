#ifndef fhatos_kernel__abstract_task_hpp
#define fhatos_kernel__abstract_task_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

template <typename TASK> class AbstractTask : public IDed, public TASK {

public:
  AbstractTask(const ID id) : IDed(id), TASK() {}
  virtual void setup();
  virtual void start();
  virtual void stop();
  virtual bool running();
  virtual void loop();
  virtual void delay(const uint16_t milliseconds);
  virtual void yield();
};

struct Thread {};

struct Coroutine {};

struct Fiber {};

} // namespace fhatos::kernel

#endif