#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include <fhatos.hpp>
///
#include <process/abstract_scheduler.hpp>
#include <process/process.hpp>
#include <process/router/local_router.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {
  class Scheduler : public AbstractScheduler {
  public:
    static Scheduler *singleton() {
      static Scheduler scheduler = Scheduler();
      return &scheduler;
    }


    const int count(const Pattern &processPattern = Pattern("#")) const override {
      if (processPattern.equals(Pattern("#")))
        return THREADS.size() + FIBERS->size() + COROUTINES.size() +
               KERNELS.size();
      std::atomic<int> *counter = new std::atomic(0);
      THREADS.forEach([counter, processPattern](const Thread *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      FIBERS->forEach([counter, processPattern](const Fiber *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      COROUTINES.forEach([counter, processPattern](const Coroutine *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      KERNELS.forEach([counter, processPattern](const KernelProcess *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      const int temp = counter->load();
      delete counter;
      return temp;
    }

    void destroy(const Pattern &processPattern) override {
      DESTROY_MUTEX.lockUnlock<bool>([this, processPattern]() {
        THREADS.remove_if([processPattern, this](Thread *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "Thread destroyed");
            delete process;
            return true;
          }
          return false;
        });
        FIBERS->remove_if([processPattern, this](Fiber *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "Fiber destroyed");
            delete process;
            return true;
          }
          return false;
        });
        COROUTINES.remove_if([processPattern, this](Coroutine *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "Coroutine destroyed");
            delete process;
            return true;
          }
          return false;
        });
        return true;
      });
      LOG_TASK(INFO, this,
               "!RFree memory after destroying !M%s!!\n"
               "\t!bsketch: %i!!\n"
               "\t!bheap  : %i!!\n"
               "\t!bpsram : %i!!\n",
               processPattern.toString().c_str(),
               ESP.getFreeSketchSpace(), ESP.getFreeHeap(), ESP.getFreePsram());
    }

    const bool spawn(Process *process) override {
      process->setup();
      if (!process->running()) {
        LOG_TASK(ERROR, this, "!RUnable to spawn running %s: %s!!\n",
                 P_TYPE_STR(process->type), process->id().toString().c_str());
        return false;
      }
      //////////////////////////////////////////////////
      ////// THREAD //////
      bool success = false;
      if (THREAD == process->type) {
        THREADS.push_back(static_cast<Thread *>(process));
        const BaseType_t threadResult = xTaskCreatePinnedToCore(
          THREAD_FUNCTION, // Function that should be called
          process->id()
          .user()
          .value_or(process->id().toString())
          .c_str(), // Name of the task (for debugging)
          10000, // Stack size (bytes)
          process, // Parameter to pass
          CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
          &static_cast<Thread *>(process)->handle, // Task handle
          tskNO_AFFINITY); // Processor core
        success = pdPASS == threadResult;
        LOG_TASK(success ? INFO : ERROR, this, "%s thread spawned", process->id().toString().c_str());
      }
      ////// FIBER //////
      else if (FIBER == process->type) {
        if (!FIBER_THREAD_HANDLE) {
          FIBERS->push_back(static_cast<Fiber *>(process));
          success = pdPASS == xTaskCreatePinnedToCore(
                      FIBER_FUNCTION, // Function that should be called
                      this->id().extend("fiber_bundle").toString().c_str(), // Name of the task (for debugging)
                      15000, // Stack size (bytes)
                      nullptr, // Parameter to pass
                      CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                      &FIBER_THREAD_HANDLE, // Task handle
                      tskNO_AFFINITY); // Processor core
        } else {
          success = FIBERS->push_back(static_cast<Fiber *>(process));
        }
        LOG_TASK(success ? INFO : ERROR, this, "%s fiber spawned", process->id().toString().c_str());
      }
      ////// COROUTINE //////
      else if (COROUTINE == process->type) {
        LOG_TASK(INFO, this, "%s coroutine spawned", process->id().toString().c_str());
        success = COROUTINES.push_back(reinterpret_cast<Coroutine *>(process));
      }
      ////// KERNEL //////
      else if (KERNEL == process->type) {
        LOG_TASK(INFO, this, "%s kernel process spawned", process->id().toString().c_str());
        return KERNELS.push_back(reinterpret_cast<KernelProcess *>(process));
      } else {
        LOG_TASK(ERROR, this, "!m%s!! has an unknown process type: !r%i!!\n",
                 process->id().toString().c_str(), process->type);
        success = false;
      }
      /*
   LOG(INFO, "!B[Scheduler Configuration]!!\n");
         LOG(NONE,
             "\tNumber of processes   : %i\n"
             "\t -Number of kernels   : %i\n"
             "\t -Number of threads   : %i\n"
             "\t -Number of fibers    : %i\n"
             "\t -Number of coroutines: %i\n",
             THREADS.size() + FIBERS.size() + KERNELS.size() + COROUTINES.size(),
             KERNELS.size(), THREADS.size(), FIBERS.size(), COROUTINES.size());
         ROUTER::singleton()->publish(StringMessage(
             this->id(), this->id().query(emptyString),
             mapString<String, MutexDeque<IDed *> *>(this->query({})),
             RETAIN_MESSAGE));
      */
      LOG(success ? INFO : ERROR,
          "!RFree memory after spawning %s !M%s!!\n"
          "\t!b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR "][flash: "
          FOS_BYTES_MB_STR "]\n",
          P_TYPE_STR(process->type),
          process->id().toString().c_str(),
          FOS_BYTES_MB(ESP.getFreeSketchSpace()),
          FOS_BYTES_MB(ESP.getFreeHeap()),
          FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));
      return success;
    }

  private:
    Scheduler() = default;

    TaskHandle_t FIBER_THREAD_HANDLE = nullptr;
    MutexDeque<Coroutine *> COROUTINES;
    MutexDeque<Fiber *> *FIBERS = new MutexDeque<Fiber *>();
    MutexDeque<Thread *> THREADS;
    MutexDeque<KernelProcess *> KERNELS;
    Mutex DESTROY_MUTEX;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *voidptr) {
      auto *fibers = Scheduler::singleton()->FIBERS;
      int counter = 0;
      while (true) {
        if (fibers->empty())
          break;
        fibers->forIndexed(counter, [counter](Fiber *fiber) {
          /////
          if (!fiber->running())
            Scheduler::singleton()->destroy(fiber->id());
          else
            fiber->loop();
        });
        vTaskDelay(1); // feeds the watchdog for the task
        counter = (counter + 1) % fibers->size();
      }
      // LOG(INFO, "!MDisconnecting master lean thread!!\n");
      vTaskDelete(nullptr);
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto *thread = (Thread *) vptr_thread;
      while (thread->running()) {
        thread->loop();
        vTaskDelay(1); // feeds the watchdog for the task
      }
      Scheduler::singleton()->destroy(thread->id());
      vTaskDelete(nullptr);
    }
  };
} // namespace fhatos

#endif
