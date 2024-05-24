#if defined(NATIVE)
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include <fhatos.hpp>
///
#include <process/abstract_scheduler.hpp>

namespace fhatos {
  class Scheduler final : public AbstractScheduler {
  public:
    static Scheduler *singleton() {
      static Scheduler scheduler = Scheduler();
      return &scheduler;
    }

    bool spawn(Process *process) override {
      // TODO: have constructed processes NOT running or check is process ID already in scheduler
      process->setup();
      if (!process->running()) {
        LOG(ERROR, "!RUnable to spawn running %s: %s!!\n",
            P_TYPE_STR(process->type), process->id().toString().c_str());
        return false;
      }
      //////////////////////////////////////////////////
      ////// THREAD //////
      bool success = false;
      switch (process->type) {
        case THREAD: {
          THREADS->push_back(static_cast<Thread *>(process));
          static_cast<Thread *>(process)->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, (void *) process);
          success = true;
          break;
        }
        case FIBER: {
          success = FIBERS->push_back(static_cast<Fiber *>(process));
          LOG(INFO, "Fiber bundle count: %i\n", FIBERS->size());
          if (!FIBER_THREAD_HANDLE) {
            FIBER_THREAD_HANDLE = new std::thread(&Scheduler::FIBER_FUNCTION, nullptr);
            if (FIBER_THREAD_HANDLE)
              success = true;
          }
          static_cast<Fiber *>(process)->xthread = FIBER_THREAD_HANDLE;
          break;
        }
        case COROUTINE: {
          success = COROUTINES->push_back(static_cast<Coroutine *>(process));
          break;
        }
        case KERNEL: {
          success = KERNELS->push_back(static_cast<KernelProcess *>(process));
          break;
        }
        default: {
          LOG(ERROR, "!m%s!! has an unknown process type: !r%i!!\n",
              process->id().toString().c_str(), process->type);
          return false;
        }
      }
      LOG(success ? INFO : ERROR, "!M%s!! %s spawned\n",
          process->id().toString().c_str(),
          P_TYPE_STR(process->type));
      /*LOG(NONE,
          "\t!yFree memory\n"
          "\t  !b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR "][flash: "
          FOS_BYTES_MB_STR "]\n",
          FOS_BYTES_MB(ESP.getFreeSketchSpace()),
          FOS_BYTES_MB(ESP.getFreeHeap()),
          FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));*/
      return success;
    }

  private:
    Scheduler() = default;

    std::thread *FIBER_THREAD_HANDLE = nullptr;
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *voidptr) {
      auto *fibers = Scheduler::singleton()->FIBERS;
      int counter = 0;
      while (!fibers->empty()) {
        Option<Fiber *> fiber = fibers->get(counter);
        if (fiber.has_value()) {
          if (!fiber.value()->running())
            Scheduler::singleton()->destroy(fiber.value()->id());
          else
            fiber.value()->loop();
          counter = (counter + 1) % fibers->size();
        } else {
          counter = 0;
        }
      }
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto *thread = static_cast<Thread *>(vptr_thread);
      while (thread->running()) {
        thread->loop();
      }
      Scheduler::singleton()->destroy(thread->id());
    }
  };
} // namespace fhatos
#endif
#endif
