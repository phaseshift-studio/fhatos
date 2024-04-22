#ifndef fhatos_kernel__scheduler_hpp
#define fhatos_kernel__scheduler_hpp

#include <fhatos.hpp>
#include <kernel/process/abstract_scheduler.hpp>
#include <kernel/process/esp32/fiber.hpp>
#include <kernel/process/esp32/thread.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

class Scheduler : AbstractScheduler<Thread, Fiber> {

protected:
  bool initialized = false;

public:
  static Scheduler *singleton() {
    static Scheduler scheduler = Scheduler();
    return &scheduler;
  }

  const bool removeThread(const ID &threadId) override {
    uint16_t size = __THREADS.size();
    __THREADS.remove_if([threadId](Thread* thread) {
      if (threadId.equals(thread->id())) {
        thread->stop();
        return true;
      }
      return false;
    });
    return __THREADS.size() < size;
  };

  const bool removeFiber(const ID &fiberId) override {
    uint16_t size = __FIBERS.size();
    for (const auto &fiber : __FIBERS) {
      if (fiber->id().equals(fiberId))
        fiber->stop();
    }
  };

  const bool addThread(Thread *thread) {
    thread->setup();
    __THREADS.push_back(thread);
    return true;
  };

  const bool addFiber(Fiber *fiber) {
    fiber->setup();
    __FIBERS.push_back(fiber);
    return true;
  };

  void setup() {
    if (!initialized) {
      initialized = true;
      // LOG(INFO, "Initializing thread pool (!rthreads:%i!!)\n",
      //    Scheduler->__THREADS.size());
      // HANDLE FIBERS
      const BaseType_t fiberResult = xTaskCreatePinnedToCore(
          __FIBER_FUNCTION, // Function that should be called
          "fibers",         // Name of the task (for debugging)
          10000,            // Stack size (bytes)
          &__FIBERS,        // Parameter to pass
          CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
          NULL,                                   // Task handle
          tskNO_AFFINITY);                        // Processor core
      // LOG(fiberResult == pdPASS ? INFO : ERROR,
      //    "!MThreading master lean task!!\n");

      // HANDLE THREADS
      for (const auto &thread : this->__THREADS) {
        const BaseType_t threadResult = xTaskCreatePinnedToCore(
            __THREAD_FUNCTION,               // Function that should be called
            thread->id().segment(1).c_str(), // Name of the task (for debugging)
            10000,                           // Stack size (bytes)
            thread,                          // Parameter to pass
            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
            &(thread->handle),                      // Task handle
            tskNO_AFFINITY);                        // Processor core
        // LOG(threadResult == pdPASS ? INFO : ERROR, "!MThreading %s %s!!\n",
        //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
      }
      /*LOG(INFO, "!B[Scheduler Configuration]!!\n");
      LOG(INFO,
          "\tNumber of threads         : %i\n"
          "\tNumber of standard threads: %i\n"
          "\tNumber of virtual threads : %i\n",
          Scheduler->__THREADS.size(),
          Scheduler->__THREADS.size() - Scheduler->__LEAN_THREADS->size(),
          Scheduler->__LEAN_THREADS->size());*/
    } else {
      // LOG(ERROR, "Thread pool already initialized via global
      // Thread::init()\n");
    }
  }

  void loop() {}

private:
  Scheduler(){};
  TaskHandle_t *FIBER_THREAD = nullptr;
  static List<Fiber *> __FIBERS;
  static List<Thread *> __THREADS;
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  static void __FIBER_FUNCTION(void *vptr_fibers) {
    List<Fiber *> *fibers = (List<Fiber *> *)vptr_fibers;
    while (!fibers->empty()) {
      fibers->remove_if([](Fiber *fiber) {
        if (!fiber->running()) {
          // LOG(INFO, "!MDisconnecting lean thread %s %s!!\n",
          //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
          fiber->stop();
          return true;
        } else {
          return false;
        }
      });
      for (Fiber *fiber : *fibers) {
        fiber->loop();
        vTaskDelay(1); // feeds the watchdog for the task
        fiber->yield();
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