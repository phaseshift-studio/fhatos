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
  template<typename ROUTER = LocalRouter<> >
  class Scheduler : public AbstractScheduler {
  public:
    static Scheduler *singleton() {
      static Scheduler scheduler = Scheduler();
      return &scheduler;
    }

    const int count(const Pattern &processPattern = Pattern("#")) const override {
      if (processPattern.equals(Pattern("#")))
        return THREADS.size() + FIBERS.size() + COROUTINES.size() +
               KERNELS.size();
      std::atomic<int> *counter = new std::atomic(0);
      THREADS.forEach([counter, processPattern](const Thread *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      FIBERS.forEach([counter, processPattern](const Fiber *process) {
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
        FIBERS.remove_if([processPattern, this](Fiber *process) {
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
      LOG_TASK(INFO, this, "!RFree memory after %s destory!! [sketch:%i][heap:%i][psram:%i]",
               processPattern.toString().c_str(), ESP.getFreeSketchSpace(), ESP.getFreeHeap(), ESP.getFreePsram());
    }

    const bool spawn(Process *process) override {
      process->setup();
      if (!process->running()) {
        LOG_TASK(ERROR, this, "!RUnable to spawn running %s!!\n",
                 P_TYPE_STR(process->type));

        return false;
      }
      //////////////////////////////////////////////////
      ////// THREAD //////
      bool success = false;
      if (THREAD == process->type) {
        THREADS.push_back(reinterpret_cast<Thread *>(process));
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
        LOG_TASK(threadResult == pdPASS ? INFO : ERROR, process, "Spawned as thread");
        success = pdPASS == threadResult;
      }
      ////// FIBER //////
      else if (FIBER == process->type) {
        BaseType_t fiberResult = pdPASS;
        if (!FIBER_THREAD) {
          FIBERS.push_back(reinterpret_cast<Fiber *>(process));
          fiberResult = xTaskCreatePinnedToCore(
            FIBER_FUNCTION, // Function that should be called
            "fibers", // Name of the task (for debugging)
            15000, // Stack size (bytes)
            &FIBERS, // Parameter to pass
            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
            FIBER_THREAD, // Task handle
            tskNO_AFFINITY); // Processor core
        } else {
          FIBERS.push_back(reinterpret_cast<Fiber *>(process));
        }
        const bool result = pdPASS == fiberResult;
        LOG_TASK(result ? INFO : ERROR, process, "Spawned as fiber",
                 process->id().toString().c_str());
        success = result;
      }
      ////// COROUTINE //////
      else if (COROUTINE == process->type) {
        LOG_TASK(INFO, process, "Spawned as coroutine");
        success = COROUTINES.push_back(reinterpret_cast<Coroutine *>(process));
      }
      ////// KERNEL //////
      else if (KERNEL == process->type) {
        LOG_TASK(INFO, process, "Spawned as kernel");
        return KERNELS.push_back(reinterpret_cast<KernelProcess *>(process));
      } else {
        LOG_TASK(ERROR, process, "!m%s!! has an unknown process type: !r%i!!\n",
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
      LOG_TASK(INFO, this, "!RFree memory after %s spawn!! [sketch:%i][heap:%i][psram:%i]", P_TYPE_STR(process->type),
               ESP.getFreeSketchSpace(), ESP.getFreeHeap(), ESP.getFreePsram());
      return success;
    }

  private:
    Scheduler() : AbstractScheduler() { this->spawn(this); };
    TaskHandle_t *FIBER_THREAD = nullptr;
    MutexDeque<Coroutine *> COROUTINES;
    MutexDeque<Fiber *> FIBERS;
    MutexDeque<Thread *> THREADS;
    MutexDeque<KernelProcess *> KERNELS;
    Mutex DESTROY_MUTEX;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *vptr_fibers) {
      auto *fibers = (MutexDeque<Fiber *> *) vptr_fibers;
      while (!fibers->empty()) {
        fibers->forEach([](Fiber *fiber) {
          if (!fiber->running()) {
            Scheduler::singleton()->destroy(fiber->id());
          } else {
            fiber->loop();
            vTaskDelay(1); // feeds the watchdog for the task
            // fiber->yield();
          }
        });
        vTaskDelay(1); // feeds the watchdog for the task
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
