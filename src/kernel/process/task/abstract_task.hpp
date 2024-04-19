#ifndef fhatos_kernel__abstract_task_hpp
#define fhatos_kernel__abstract_task_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

template <typename TASK> class Task : public TASK {

public:
  Task(const ID id) : TASK(id) {}
  virtual void start();
  virtual void stop();
  virtual bool running();
  virtual void loop();
  virtual void delay(const uint16_t milliseconds);
  virtual void yield();
};

} // namespace fhatos::kernel

#endif