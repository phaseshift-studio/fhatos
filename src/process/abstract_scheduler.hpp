#ifndef fhatos_abstract_scheduler_hpp
#define fhatos_abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <process/process.hpp>
#include <structure/io/net/f_wifi.hpp>
#include <structure/furi.hpp>
//
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {
  class AbstractScheduler : public IDed {
  protected:
    AbstractScheduler(const ID &id = ID("scheduler")) : IDed(id) {
    }

  public:
    virtual const bool spawn(Process *process) { return false; }

    virtual void destroy(const Pattern &processPattern) {
    }

    virtual const int count(const Pattern &processPattern = Pattern("#")) const { return 0; }
  };
} // namespace fhatos

#endif
