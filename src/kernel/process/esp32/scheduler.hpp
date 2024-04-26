#ifndef fhatos_kernel__scheduler_hpp
#define fhatos_kernel__scheduler_hpp

#include <fhatos.hpp>
///
#include <kernel/process/abstract_scheduler.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

class Scheduler : public AbstractScheduler {

protected:
  bool initialized = false;

public:
  static Scheduler *singleton() {
    static Scheduler scheduler = Scheduler();
    return &scheduler;
  }

  const bool removeThread(const ID &threadId) {
    uint16_t size = __THREADS.size();
    __THREADS.remove_if([threadId](Thread *thread) {
      if (threadId.equals(thread->id())) {
        thread->stop();
        return true;
      }
      return false;
    });
    return __THREADS.size() < size;
  };

  const bool removeFiber(const ID &fiberId) {
    uint16_t size = __FIBERS.size();
    for (const auto &fiber : __FIBERS) {
      if (fiber->id().equals(fiberId))
        fiber->stop();
    }
    return __FIBERS.size() < size;
  };

  const bool removeCoroutine(const ID &coroutineId) {
    uint16_t size = __COROUTINES.size();
    for (const auto &coroutine : __COROUTINES) {
      if (coroutine->id().equals(coroutineId))
        coroutine->stop();
    }
    return __COROUTINES.size() < size;
  };

  const bool addProcess(Thread *thread) {
    thread->setup();
    __THREADS.push_back(thread);
    return true;
  };

  const bool addProcess(Fiber *fiber) {
    fiber->setup();
    __FIBERS.push_back(fiber);
    return true;
  };

  const bool addProcess(Coroutine *coroutine) {
    coroutine->setup();
    __COROUTINES.push_back(coroutine);
    return true;
  };

  const bool addProcess(KernelProcess *kernelProcess) {
    kernelProcess->setup();
    __KERNELS.push_back(kernelProcess);
    return true;
  };

  void setup() {
    if (!initialized) {
      initialized = true;
      LOG(INFO, "Initializing thread pool (!rthreads:%i!!)\n",
          __THREADS.size());
      // HANDLE FIBERS
      const BaseType_t fiberResult = xTaskCreatePinnedToCore(
          __FIBER_FUNCTION, // Function that should be called
          "fibers",         // Name of the task (for debugging)
          10000,            // Stack size (bytes)
          &__FIBERS,        // Parameter to pass
          CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
          NULL,                                   // Task handle
          tskNO_AFFINITY);                        // Processor core
      if (fiberResult != pdPASS)
        LOG(ERROR, "!MStarting master fiber thread!!\n");

      // HANDLE THREADS
      for (const auto &thread : __THREADS) {
        const BaseType_t threadResult = xTaskCreatePinnedToCore(
            __THREAD_FUNCTION, // Function that should be called
            thread->id()
                .user()
                .value_or(thread->id().toString())
                .c_str(), // Name of the task (for debugging)
            10000,        // Stack size (bytes)
            thread,       // Parameter to pass
            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
            &(thread->handle),                      // Task handle
            tskNO_AFFINITY);                        // Processor core
        LOG(threadResult == pdPASS ? INFO : ERROR, "!MStarting thread %s!!\n",
            thread->id().toString().c_str());
      }
      LOG(INFO, "!B[Scheduler Configuration]!!\n");
      LOG(INFO,
          "\tNumber of processes   : %i\n"
          "\t  Number of kernels   : %i\n"
          "\t  Number of threads   : %i\n"
          "\t  Number of fibers    : %i\n"
          "\t  Number of coroutines: %i\n",
          __THREADS.size() + __FIBERS.size() + __KERNELS.size() +
              __COROUTINES.size(),
          __KERNELS.size(), __THREADS.size(), __FIBERS.size(),
          __COROUTINES.size());
    } else {
      LOG(ERROR,
          "Thread pool already initialized via global Scheduler::setup()\n");
    }
  }

  void loop() {}

private:
  Scheduler(){};
  TaskHandle_t *FIBER_THREAD = nullptr;
  List<Coroutine *> __COROUTINES;
  List<Fiber *> __FIBERS;
  List<Thread *> __THREADS;
  List<KernelProcess *> __KERNELS;
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  static void __FIBER_FUNCTION(void *vptr_fibers) {
    List<Fiber *> *fibers = (List<Fiber *> *)vptr_fibers;
    for (const auto &fiber : *fibers) {
      LOG(INFO, "!MStarting fiber %s!!\n", fiber->id().toString().c_str());
    }
    while (!fibers->empty()) {
      for (const auto &fiber : *fibers) {
        if (!fiber->running()) {
          LOG(INFO, "!MStopping fiber %s!!\n", fiber->id().toString().c_str());
          fibers->remove(fiber);
          delete fiber;
        } else {
          fiber->loop();
          vTaskDelay(1); // feeds the watchdog for the task
          // fiber->yield();
        }
      }
      vTaskDelay(1); // feeds the watchdog for the task
    }
    // LOG(INFO, "!MDisconnecting master lean thread!!\n");
    vTaskDelete(NULL);
  }
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  static void __THREAD_FUNCTION(void *vptr_thread) {
    Thread *thread = (Thread *)vptr_thread;
    while (thread->running()) {
      thread->loop();
      vTaskDelay(1); // feeds the watchdog for the task
    }
    // LOG(INFO, "!MDisconnecting thread %s %s!!\n",
    //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
    Scheduler::singleton()->removeThread(thread->id());
    vTaskDelete(NULL);
  }
};

} // namespace fhatos::kernel

#endif