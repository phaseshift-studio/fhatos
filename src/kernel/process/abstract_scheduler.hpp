#ifndef fhatos_kernel__abstract_scheduler_hpp
#define fhatos_kernel__abstract_scheduler_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename THREAD, typename FIBER> class AbstractScheduler {

public:
  virtual const bool addThread(THREAD &thread);
  virtual const bool addFiber(FIBER &fiber);
  virtual const bool removeThread(const ID &threadId);
  virtual const bool removeFiber(const ID &fiberId);
  virtual void setup();
  virtual void loop();
};

} // namespace fhatos::kernel

#endif