#ifndef fhatos_abstract_scheduler_hpp
#define fhatos_abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <process/process.hpp>
#include <structure/io/net/f_wifi.hpp>
#include <furi.hpp>
//
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {

class AbstractScheduler : public Coroutine {

protected:
  AbstractScheduler() : Coroutine(fWIFI::idFromIP("kernel", "scheduler")) {}

public:
  virtual const bool spawn(Process *process) { return false; }

  virtual void destroy(const Pattern &processPattern) {}

  virtual const int count(const Pattern &processPattern = Pattern("#")) const { return 0; }
};

} // namespace fhatos

#endif