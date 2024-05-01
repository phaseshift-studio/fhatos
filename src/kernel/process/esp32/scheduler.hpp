#ifndef fhatos_kernel_scheduler_hpp
#define fhatos_kernel_scheduler_hpp

#include <fhatos.hpp>
///
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

  const uint8_t threadCount() const override { return THREADS.size(); }

  bool removeThread(const ID &threadId) override {
    const uint16_t size = THREADS.size();
    THREADS.remove_if([threadId, this](Thread *thread) {
      if (thread->id().equals(threadId)) {
        thread->stop();
        return true;
      }
      return false;
    });
    return THREADS.size() < size;
  };

  bool removeFiber(const ID &fiberId) override {
    const uint16_t size = FIBERS.size();
    FIBERS.remove_if([fiberId, this](Fiber *fiber) {
      if (fiber->id().equals(fiberId)) {
        fiber->stop();
        return true;
      }
      return false;
    });
    return FIBERS.size() < size;
  };

  bool removeCoroutine(const ID &coroutineId) override {
    uint16_t size = COROUTINES.size();
    /* for (const auto &coroutine : COROUTINES) {
       if (coroutine->id().equals(coroutineId))
         coroutine->stop();
     }*/
    return COROUTINES.size() < size;
  };

  bool addProcess(Thread *thread) override {
    thread->setup();
    return thread->running() ? THREADS.push_back(thread) : false;
  }

  bool addProcess(Fiber *fiber) override {
    fiber->setup();
    return fiber->running() ? FIBERS.push_back(fiber) : false;
  };

  bool addProcess(Coroutine *coroutine) override {
    LOG(INFO, "!MStarting coroutine %s!!\n",
        coroutine->id().toString().c_str());
    coroutine->setup();
    return coroutine->running() ? COROUTINES.push_back(coroutine) : false;
  };

  bool addProcess(KernelProcess *kernelProcess) {
    kernelProcess->setup();
    return kernelProcess->running() ? KERNELS.push_back(kernelProcess) : false;
  };

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

  void setup() override {
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
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
      /////////////////////////////////////////////////////////////////////////
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
      List<Thread *> *COPY = new List<Thread *>();
      THREADS.forEach([COPY](Thread *thread) { COPY->push_back(thread); });
      for (const auto &thread : *COPY) {
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
    } else {
      LOG(ERROR, "Scheduler processes already initialized via global "
                 "Scheduler::setup()\n");
    }
  }

  void loop() override {}

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
    fibers->forEach([](Fiber *fiber) {
      LOG(INFO, "!MStarting fiber %s!!\n", fiber->id().toString().c_str());
    });
    while (!fibers->empty()) {
      fibers->forEach([fibers](Fiber *fiber) {
        if (!fiber->running()) {
          LOG(INFO, "!MStopping fiber %s!!\n", fiber->id().toString().c_str());
          fibers->remove(fiber);
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
    // LOG(INFO, "!MDisconnecting thread %s %s!!\n",
    //     Helper::typeName(xthread).c_str(), xthread->id().c_str());
    Scheduler::singleton()->removeThread(thread->id());
    delete thread;
    vTaskDelete(nullptr);
  }
};

} // namespace fhatos::kernel

#endif