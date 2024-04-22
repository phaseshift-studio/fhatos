#ifndef fhatos_kernel__abstract_scheduler_hpp
#define fhatos_kernel__abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

class AbstractScheduler {

public:
  virtual const bool addProcess(const KernelProcess *kernelProcess) {
    return true;
  }
  virtual const bool addProcess(Thread *thread) { return true; };
  virtual const bool addProcess(Fiber *fiber) { return true; };
  virtual const bool addProcess(Coroutine *coroutine) { return true; };
  virtual const bool removeThread(const ID &threadId) { return true; };
  virtual const bool removeFiber(const ID &fiberId) { return true; };
  virtual const bool removeCoroutine(const ID &coroutineId) { return true; }
  virtual void setup(){};
  virtual void loop(){};
};

} // namespace fhatos::kernel

#endif