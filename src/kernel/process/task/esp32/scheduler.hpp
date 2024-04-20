#ifndef fhatos_kernel__scheduler_hpp
#define fhatos_kernel__scheduler_hpp

#include <fhatos.hpp>
#include <kernel/process/task/abstract_scheduler.hpp>
#include <kernel/process/task/esp32/task.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

class Scheduler {

public:
  static Scheduler *singleton() {
    static Scheduler scheduler = Scheduler();
    return &scheduler;
  }
  static void init() {
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      // LOG(INFO, "Initializing thread pool (!rthreads:%i!!)\n",
      //    Scheduler->__THREADS.size());
      for (const auto &vptr_task : Scheduler::singleton()->__THREADS) {
        Task<Thread> *task = (Task<Thread> *)vptr_task;
        if (true) { // task->isLean()) {
          if (Scheduler::singleton()->FIBER_THREAD) {
            Scheduler::singleton()->__FIBERS->push_back(task);
            //   LOG(INFO, "!MVirtual Threading %s %s!!\n",
            //      Helper::typeName(xthread).c_str(), xthread->id().c_str());
          } else {
            Scheduler::singleton()->__FIBERS = new List<void *>{task};
            const BaseType_t result = xTaskCreatePinnedToCore(
                __FIBER_FUNCTION,      // Function that should be called
                "Master fiber thread", // Name of the task (for debugging)
                10000,                 // Stack size (bytes)
                Scheduler::singleton()->__FIBERS,       // Parameter to pass
                CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                NULL,                                   // Task handle
                tskNO_AFFINITY);                        // Processor core
            // LOG(result == pdPASS ? INFO : ERROR,
            //    "!MThreading master lean task!!\n");
          }
        } else {
          const BaseType_t result = xTaskCreatePinnedToCore(
              __THREAD_FUNCTION,             // Function that should be called
              task->id().segment(1).c_str(), // Name of the task (for debugging)
              10000,                         // Stack size (bytes)
              task,                          // Parameter to pass
              CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
              &(task->handle),                        // Task handle
              tskNO_AFFINITY);                        // Processor core
          // LOG(result == pdPASS ? INFO : ERROR, "!MThreading %s %s!!\n",
          //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
        }
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

private:
  Scheduler(){};
  TaskHandle_t *FIBER_THREAD = nullptr;
  List<void *> *__FIBERS;
  List<void *> __THREADS;
  static void removeTask(Task<Thread> *task) {
    Scheduler::singleton()->__THREADS.remove_if([task](void *tempTask) {
      return task->equals(*(Task<Thread> *)tempTask);
    });
  }
  static void __FIBER_FUNCTION(void *vptr_fibers) {
    List<Task<Fiber> *> *fibers = (List<Task<Fiber> *> *)vptr_fibers;
    while (!fibers->empty()) {
      fibers->remove_if([](Task<Fiber> *fiber) {
        if (!fiber->running()) {
          // LOG(INFO, "!MDisconnecting lean thread %s %s!!\n",
          //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
          fiber->stop();
          return true;
        } else {
          return false;
        }
      });
      for (const auto &fiber : *fibers) {
        fiber->loop();
        vTaskDelay(1); // feeds the watchdog for the task
        fiber->yield();
      }
      vTaskDelay(1); // feeds the watchdog for the task
    }
    // LOG(INFO, "!MDisconnecting master lean thread!!\n");
    vTaskDelete(NULL);
  }
  ////////////////////////////////////////////
  static void __THREAD_FUNCTION(void *vptr_thread) {
    Task<Thread> *thread = (Task<Thread> *)vptr_thread;
    while (thread->running()) {
      thread->loop();
      vTaskDelay(1); // feeds the watchdog for the task
    }
    // LOG(INFO, "!MDisconnecting thread %s %s!!\n",
    //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
    Scheduler::removeTask(thread);
    vTaskDelete(NULL);
  }
};

} // namespace fhatos::kernel

#endif