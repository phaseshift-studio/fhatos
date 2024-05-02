#ifndef fhatos_kernel__abstract_scheduler_hpp
#define fhatos_kernel__abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/local_router/local_router.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>
#include <kernel/structure/routes.hpp>
#include <kernel/structure/structure.hpp>
//
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

class AbstractScheduler : public Coroutine {

protected:
  AbstractScheduler() : Coroutine(fWIFI::idFromIP("kernel", "scheduler")) {}

public:
  virtual const bool spawn(Process *process) { return false; }

  virtual void destroy(const Pattern &processPattern) {}

  virtual const int count(const Pattern &processPattern) const { return 0; }
};

} // namespace fhatos::kernel

#endif