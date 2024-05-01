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
   std::atomic<int>* counter = new std::atomic(0);
    THREADS.forEach([counter, processPattern](Thread *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load()+1);
    });
    FIBERS.forEach([counter, processPattern](Fiber *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load()+1);
    });
    COROUTINES.forEach([counter, processPattern](Coroutine *process) {
      if (process->id().matches(processPattern))
       counter->store(counter->load()+1);
    });
    KERNELS.forEach([counter, processPattern](KernelProcess *process) {
      if (process->id().matches(processPattern))
        counter->store(counter->load()+1);
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
    const char *processType = typeid(*process).name();
    if (strstr(processType, "Thread") == 0) {
      THREADS.push_back(reinterpret_cast<Thread *>(process));
    } else if (strstr(processType, "Fiber") == 0) {
      FIBERS.push_back(reinterpret_cast<Fiber *>(process));
    } else if (strstr(processType, "Coroutine") == 0) {
      COROUTINES.push_back(reinterpret_cast<Coroutine *>(process));
    } else if (strstr(processType, "KernelProcess") == 0) {
      KERNELS.push_back(reinterpret_cast<KernelProcess *>(process));
    } else {
      throw fError("Unknown process type: %s", processType);
    }
    return true;
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
    Scheduler::singleton()->destroy(thread->id());
    delete thread;
    vTaskDelete(nullptr);
  }
};

} // namespace fhatos::kernel

#endif