#ifndef fhatos_kernel_scheduler_hpp
#define fhatos_kernel_scheduler_hpp

#include <fhatos.hpp>
///
#include <kernel/process/abstract_scheduler.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

class Scheduler : public AbstractScheduler {

public:
  static Scheduler *singleton() {
    static Scheduler scheduler = Scheduler();
    return &scheduler;
  }

  bool removeThread(const ID &threadId) override {
    uint16_t size = THREADS.size();
    THREADS.remove_if([threadId](Thread *thread) {
      if (threadId.equals(thread->id())) {
        thread->stop();
        return true;
      }
      return false;
    });
    return THREADS.size() < size;
  };

  bool removeFiber(const ID &fiberId) override {
    uint16_t size = FIBERS.size();
    for (const auto &fiber : FIBERS) {
      if (fiber->id().equals(fiberId))
        fiber->stop();
    }
    return FIBERS.size() < size;
  };

  bool removeCoroutine(const ID &coroutineId) override {
    uint16_t size = COROUTINES.size();
    for (const auto &coroutine : COROUTINES) {
      if (coroutine->id().equals(coroutineId))
        coroutine->stop();
    }
    return COROUTINES.size() < size;
  };

  bool addProcess(Thread *thread) override {
    thread->setup();
    THREADS.push_back(thread);
    return true;
  };

  bool addProcess(Fiber *fiber) override {
    fiber->setup();
    FIBERS.push_back(fiber);
    return true;
  };

  bool addProcess(Coroutine *coroutine) override {
    LOG(INFO, "!MStarting coroutine %s!!\n",
        coroutine->id().toString().c_str());
    coroutine->setup();
    COROUTINES.push_back(coroutine);
    return true;
  };

  bool addProcess(KernelProcess *kernelProcess) {
    kernelProcess->setup();
    KERNELS.push_back(kernelProcess);
    return true;
  };

  virtual Map<String, List<IDed *> *>
  query(const Set<String> &labels) override {
    Map<String, List<IDed *> *> result;
    if ((labels.empty() || labels.count("kernel") > 0) && !KERNELS.empty()) {
      result.emplace("kernel", reinterpret_cast<List<IDed *> *>(&KERNELS));
    }
    if ((labels.empty() || labels.count("thread") > 0) && !THREADS.empty()) {
      result.emplace("thread", reinterpret_cast<List<IDed *> *>(&THREADS));
    }
    if ((labels.empty() || labels.count("fiber") > 0) && !FIBERS.empty()) {
      result.emplace("fiber", reinterpret_cast<List<IDed *> *>(&FIBERS));
    }
    if ((labels.empty() || labels.count("coroutine") > 0) &&
        !COROUTINES.empty()) {
      result.emplace("coroutine",
                     reinterpret_cast<List<IDed *> *>(&COROUTINES));
    }
    return result;
  }

  void setup() override {

    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      LOG(INFO, "Initializing thread pool (!rthreads:%i!!)\n", THREADS.size());
      // HANDLE FIBERS
      const BaseType_t fiberResult = xTaskCreatePinnedToCore(
          FIBER_FUNCTION, // Function that should be called
          "fibers",       // Name of the task (for debugging)
          10000,          // Stack size (bytes)
          &FIBERS,        // Parameter to pass
          CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
          FIBER_THREAD,                           // Task handle
          tskNO_AFFINITY);                        // Processor core
      if (fiberResult != pdPASS)
        LOG(ERROR, "!MStarting master fiber thread!!\n");

      // HANDLE THREADS
      for (const auto &thread : THREADS) {
        const BaseType_t threadResult = xTaskCreatePinnedToCore(
            THREAD_FUNCTION, // Function that should be called
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
      LOG(NONE,
          "\tNumber of processes   : %i\n"
          "\t -Number of kernels   : %i\n"
          "\t -Number of threads   : %i\n"
          "\t -Number of fibers    : %i\n"
          "\t -Number of coroutines: %i\n",
          THREADS.size() + FIBERS.size() + KERNELS.size() + COROUTINES.size(),
          KERNELS.size(), THREADS.size(), FIBERS.size(), COROUTINES.size());
      LocalRouter<StringMessage>::singleton()->publish(
          StringMessage(this->id(),
           this->id().query(""),
           mapString<String,IDed>(this->query({})),
           RETAIN_MESSAGE));
    } else {
      LOG(ERROR, "Scheduler processes already initialized via global "
                 "Scheduler::setup()\n");
    }
  }

  void loop() override {}

private:
  Scheduler() : AbstractScheduler() {};
  TaskHandle_t *FIBER_THREAD = nullptr;
  List<Coroutine *> COROUTINES;
  List<Fiber *> FIBERS;
  List<Thread *> THREADS;
  List<KernelProcess *> KERNELS;

  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  static void FIBER_FUNCTION(void *vptr_fibers) {
    auto *fibers = (List<Fiber *> *)vptr_fibers;
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
    // LOG(INFO, "!MDisconnecting thread %s %s!!\n",
    //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
    Scheduler::singleton()->removeThread(thread->id());
    vTaskDelete(nullptr);
  }
};

} // namespace fhatos::kernel

#endif