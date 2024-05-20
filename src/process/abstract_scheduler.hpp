#ifndef fhatos_abstract_scheduler_hpp
#define fhatos_abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <process/process.hpp>
#include <structure/furi.hpp>
#include <util/mutex.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  class AbstractScheduler {
    Mutex DESTROY_MUTEX;

  protected:
    MutexDeque<Coroutine *> *COROUTINES = new MutexDeque<Coroutine *>();
    MutexDeque<Fiber *> *FIBERS = new MutexDeque<Fiber *>();
    MutexDeque<Thread *> *THREADS = new MutexDeque<Thread *>();
    MutexDeque<KernelProcess *> *KERNELS = new MutexDeque<KernelProcess *>();

    AbstractScheduler() = default;

  public:
    virtual ~AbstractScheduler() {
      delete COROUTINES;
      delete FIBERS;
      delete THREADS;
      delete KERNELS;
    };

    virtual bool spawn(Process *process) { return false; }

    virtual bool destroy(const Pattern &processPattern) {
      const bool success = DESTROY_MUTEX.lockUnlock<bool>([this, processPattern]() {
        THREADS->remove_if([processPattern, this](Thread *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG(INFO, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        FIBERS->remove_if([processPattern, this](Fiber *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG(INFO, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        COROUTINES->remove_if([processPattern, this](Coroutine *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG(INFO, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        return true;
      });
      LOG(NONE,
          "\t!yFree memory\n"
          "\t  !b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR "][flash: "
          FOS_BYTES_MB_STR "]\n",
          FOS_BYTES_MB(ESP.getFreeSketchSpace()),
          FOS_BYTES_MB(ESP.getFreeHeap()),
          FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));
      return success;
    }

    List<Process *> *find(const Pattern &processPattern = Pattern("#")) const {
      List<Process *> *results = new List<Process *>();
      List<Process *> *temp = (List<Process *> *) (void *) THREADS->match([processPattern](const Process *p) {
        return p->id().matches(processPattern);
      });
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = (List<Process *> *) (void *) FIBERS->match([processPattern](const Process *p) {
        return p->id().matches(processPattern);
      });
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = (List<Process *> *) (void *) COROUTINES->match([processPattern](const Process *p) {
        return p->id().matches(processPattern);
      });
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = (List<Process *> *) (void *) KERNELS->match([processPattern](const Process *p) {
        return p->id().matches(processPattern);
      });
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      return results;
    }

    virtual int count(const Pattern &processPattern = Pattern("#")) const {
      if (processPattern.equals(Pattern("#")))
        return THREADS->size() + FIBERS->size() + COROUTINES->size() +
               KERNELS->size();
      std::atomic<int> *counter = new std::atomic(0);
      THREADS->forEach([counter, processPattern](const Thread *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      try {
        FIBERS->forEach([counter, processPattern](const Fiber *process) {
          if (process->id().matches(processPattern))
            counter->fetch_add(1);
        });
      } catch (fError e) {
        LOG(ERROR, "HERE: %s\n", e.what());
      }
      COROUTINES->forEach([counter, processPattern](const Coroutine *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      KERNELS->forEach([counter, processPattern](const KernelProcess *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      const int temp = counter->load();
      delete counter;
      return temp;
    }
  };
} // namespace fhatos

#endif
