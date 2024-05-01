#ifndef fhatos_kernel__abstract_scheduler_hpp
#define fhatos_kernel__abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/local_router/local_router.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/routes.hpp>
#include <kernel/structure/structure.hpp>
//
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

class AbstractScheduler : public Coroutine {

protected:
  AbstractScheduler() : Coroutine(WIFI::idFromIP("kernel", "scheduler")) {}

public:
  virtual bool addProcess(const KernelProcess *kernelProcess) { return true; }

  virtual bool addProcess(Thread *thread) { return true; };

  virtual bool addProcess(Fiber *fiber) { return true; };

  virtual bool addProcess(Coroutine *coroutine) { return true; };

  virtual bool removeThread(const ID &threadId) { return true; };

  virtual bool removeFiber(const ID &fiberId) { return true; };

  virtual bool removeCoroutine(const ID &coroutineId) { return true; }

  virtual const uint8_t threadCount() const { return 0; }

  void setup() override{};

  void loop() override{};
};

} // namespace fhatos::kernel

#endif