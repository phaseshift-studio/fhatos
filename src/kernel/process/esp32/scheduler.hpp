#ifndef fhatos_kernel_scheduler_hpp
#define fhatos_kernel_scheduler_hpp

#include <fhatos.hpp>
///
#include <atomic>
#include <kernel/process/abstract_scheduler.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/util/mutex/mutex_deque.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

template <typename ROUTER = LocalRouter<Message<String>>>
class Scheduler : public AbstractScheduler {

public:
  static Scheduler *singleton() {
    static Scheduler scheduler = Scheduler();
    return &scheduler;
  }

  const int count(const Pattern &processPattern) const override {
    std::atomic<int> *counter = new std::atomic(0);
    THREADS.forEach([counter, processPattern](Thread *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load() + 1);
    });
    FIBERS.forEach([counter, processPattern](Fiber *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load() + 1);
    });
    COROUTINES.forEach([counter, processPattern](Coroutine *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load() + 1);
    });
    KERNELS.forEach([counter, processPattern](KernelProcess *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load() + 1);
    });
    const int temp = counter->load();
    delete counter;
    return temp;
  }

  void destroy(const Pattern &processPattern) {
    THREADS.remove_if([processPattern, this](Thread *process) {
      if (process->id().matches(processPattern)) {
        process->stop();
        return true;
      }
      return false;
    });
    FIBERS.remove_if([processPattern, this](Fiber *process) {
      if (process->id().matches(processPattern)) {
        process->stop();
        return true;
      }
      return false;
    });
    COROUTINES.remove_if([processPattern, this](Coroutine *process) {
      if (process->id().matches(processPattern)) {
        process->stop();
        return true;
      }
      return false;
    });
  }

  const bool spawn(Process *process) override {
    process->setup();
    if (!process->running())
      return false;
    //////////////////////////////////////////////////
    ////// THREAD //////
    if (THREAD == process->pType) {
      const BaseType_t threadResult = xTaskCreatePinnedToCore(
          THREAD_FUNCTION, // Function that should be called
          process->id()
              .user()
              .value_or(process->id().toString())
              .c_str(), // Name of the task (for debugging)
          10000,        // Stack size (bytes)
          process,      // Parameter to pass
          CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
          &(((Thread *)process)->handle),         // Task handle
          tskNO_AFFINITY);                        // Processor core
      LOG(threadResult == pdPASS ? INFO : ERROR, "!MThread %s spawned!!\n",
          process->id().toString().c_str());
      return pdPASS == threadResult &&
             THREADS.push_back(reinterpret_cast<Thread *>(process));
    }
    ////// FIBER //////
    else if (FIBER == process->pType) {
      BaseType_t fiberResult = pdFAIL;
      if (!FIBER_THREAD) {
        fiberResult = xTaskCreatePinnedToCore(
            FIBER_FUNCTION, // Function that should be called
            "fibers",       // Name of the task (for debugging)
            10000,          // Stack size (bytes)
            &FIBERS,        // Parameter to pass
            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
            FIBER_THREAD,                           // Task handle
            tskNO_AFFINITY);                        // Processor core
        if (fiberResult != pdPASS)
          LOG(ERROR, "!MFiber thread spawned!!\n");
      }
      LOG(fiberResult == pdPASS ? INFO : ERROR, "!MFiber %s spawned!!\n",
          process->id().toString().c_str());
      return pdPASS == fiberResult &&
             FIBERS.push_back(reinterpret_cast<Fiber *>(process));
    }
    ////// COROUTINE //////
    else if (COROUTINE == process->pType) {
      LOG(INFO, "!MCoroutine %s spawned!!\n", process->id().toString().c_str());
      return COROUTINES.push_back(reinterpret_cast<Coroutine *>(process));
    }
    ////// KERNEL //////
    else if (KERNEL == process->pType) {
      LOG(INFO, "!MKernel %s spawned!!\n", process->id().toString().c_str());
      return KERNELS.push_back(reinterpret_cast<KernelProcess *>(process));
    } else {
      LOG(ERROR, "!m%s!! has an unknown process type: !r%i!!\n",
          process->id().toString().c_str(), process->pType);
      return false;
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
           this->id(), this->id().query(""),
           mapString<String, MutexDeque<IDed *> *>(this->query({})),
           RETAIN_MESSAGE));
    */
  }

  virtual Map<String, MutexDeque<IDed *> *>
  query(const Set<String> &labels) override {
    Map<String, MutexDeque<IDed *> *> result;
    if ((labels.empty() || labels.count("kernel") > 0) && !KERNELS.empty()) {
      result.emplace("kernel",
                     reinterpret_cast<MutexDeque<IDed *> *>(&KERNELS));
    }
    if ((labels.empty() || labels.count("thread") > 0) && !THREADS.empty()) {
      result.emplace("thread",
                     reinterpret_cast<MutexDeque<IDed *> *>(&THREADS));
    }
    if ((labels.empty() || labels.count("fiber") > 0) && !FIBERS.empty()) {
      result.emplace("fiber", reinterpret_cast<MutexDeque<IDed *> *>(&FIBERS));
    }
    if ((labels.empty() || labels.count("coroutine") > 0) &&
        !COROUTINES.empty()) {
      result.emplace("coroutine",
                     reinterpret_cast<MutexDeque<IDed *> *>(&COROUTINES));
    }
    return result;
  }

private:
  Scheduler() : AbstractScheduler() {};
  TaskHandle_t *FIBER_THREAD = nullptr;
  MutexDeque<Coroutine *> COROUTINES;
  MutexDeque<Fiber *> FIBERS;
  MutexDeque<Thread *> THREADS;
  MutexDeque<KernelProcess *> KERNELS;

  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  static void FIBER_FUNCTION(void *vptr_fibers) {
    auto *fibers = (MutexDeque<Fiber *> *)vptr_fibers;
    while (!fibers->empty()) {
      fibers->forEach([fibers](Fiber *fiber) {
        if (!fiber->running()) {
          LOG(INFO, "!MStopping fiber %s!!\n", fiber->id().toString().c_str());
          Scheduler::singleton()->destroy(fiber->id());
          delete fiber;
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
    auto *thread = (Thread *)vptr_thread;
    while (thread->running()) {
      thread->loop();
      vTaskDelay(1); // feeds the watchdog for the task
    }
    LOG(INFO, "!MStopping thread %s!!\n", thread->id().toString().c_str());
    Scheduler::singleton()->destroy(thread->id());
    delete thread;
    vTaskDelete(nullptr);
  }
};

} // namespace fhatos::kernel

#endif