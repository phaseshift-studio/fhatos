/*******************************************************************************
  FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
#pragma once
#if defined(ESP32)
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>
///
// #include <esp_heap_trace.h>
#include <process/x_scheduler.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)

#ifndef FOS_ESP_THREAD_STACK_SIZE
#define FOS_ESP_THREAD_STACK_SIZE 25000
#endif
#ifndef FOS_ESP_FIBER_STACK_SIZE
#define FOS_ESP_FIBER_STACK_SIZE 20000
#endif


// #ifdef CONFIG_HEAP_TRACING_DEST
// #define NUM_RECORDS 100
//  static heap_trace_record_t* trace_record = new heap_trace_record_t[NUM_RECORDS]; // This buffer must be in internal
//  RAM #endif

namespace fhatos {
  class Scheduler final : public XScheduler {
  public:
    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool setup_ = false;
      static ptr<Scheduler> scheduler = ptr<Scheduler>(new Scheduler(id));
      if (!setup_) {
        //scheduler_thread = make_shared<thread::id>(this_thread::get_id());
        setup_ = true;
      }
     return scheduler;
    }

    void feed_local_watchdog() override {
      vTaskDelay(1); // feeds the watchdog for the task
    }

    virtual bool spawn(const Process_p &process) override {
      process->setup();
      if (!process->running) {
        LOG_SCHEDULER(ERROR, "!b%s!! !yprocess!! failed to setup\n", process->id()->toString().c_str());
        return false;
      }
      ////////////////////////////////
      bool success = false;
      const uint16_t stack_size =
          process->type()->has_path("fiber")
              ? FOS_ESP_FIBER_STACK_SIZE
              : (process->type()->has_path("thread") && process->running ? FOS_ESP_THREAD_STACK_SIZE : 0);
      BaseType_t threadResult;
      if (process->type()->has_path("thread")) {
        threadResult = xTaskCreatePinnedToCore(THREAD_FUNCTION, // Function that should be called
                                               process->id()->toString().c_str(), // Name of the task (for debugging)
                                               stack_size, // Stack size (bytes)
                                               process.get(), // Parameter to pass
                                               CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                               &static_cast<Thread *>(process.get())->handle, // Task handle
                                               tskNO_AFFINITY); // Processor core
      } else if (process->type()->has_path("fiber")) {
        if (!FIBER_THREAD_HANDLE) {
          threadResult = xTaskCreatePinnedToCore(FIBER_FUNCTION, // Function that should be called
                                                 "fiber_bundle", // Name of the task (for debugging)
                                                 stack_size, // Stack size (bytes)
                                                 nullptr, // Parameter to pass
                                                 CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                                 &FIBER_THREAD_HANDLE, // Task handle
                                                 tskNO_AFFINITY); // Processor core
        }
      } else {
        process->running = false;
        LOG_SCHEDULER(ERROR, "!b%s!! !yprocess!! failed to spawn\n", process->id()->toString().c_str());
        return false;
      }
      success = pdPASS == threadResult;
      if (success) {
        this->processes_->push_back(process);
        LOG_SCHEDULER(INFO, "!b%s!! !yprocess!! spawned (w/ %i bytes stack)\n", process->id()->toString().c_str(),
                      stack_size);
        this->rec_get(vri("process"))->lst_add(vri(process->id()));
        router()->write(this->id(), PtrHelper::no_delete(this));
      } else {
        const char *reason = threadResult == -1 ? "COULD_NOT_ALLOCATE_REQUIRED_MEMORY" : "UNKNOWN_REASON";
        LOG_SCHEDULER(ERROR, "!b%s!! !yprocess!! failed to spawn [error:%i %s]\n", process->id()->toString().c_str(),
                      threadResult, reason);
      }
      return success;
    }


  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {
      // ESP_ERROR_CHECK(heap_trace_init_standalone(trace_record, NUM_RECORDS));
      rec_set(vri(":spawn"), to_bcode([this](const Obj_p &obj) {
             if (!obj->id())
               throw fError("value id required to spawn %s", obj->toString().c_str());
             return dool(this->spawn(make_shared<Thread>(obj)));
           }, StringHelper::cxx_f_metadata(__FILE__,__LINE__)));
    }

    TaskHandle_t FIBER_THREAD_HANDLE = nullptr;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *) {
      int counter = 1;
      while (counter > 0) {
        counter = 0;
        auto *fibers = new List<Process_p>();
        Scheduler::singleton()->processes_->forEach([fibers](const Process_p &proc) {
          if (proc->type()->has_path("fiber") && proc->running)
            fibers->push_back(proc);
        });
        for (const Process_p &fiber: *fibers) {
          if (fiber->running)
            fiber->loop();
          counter++;
        }
        Scheduler::singleton()->processes_->remove_if([](const Process_p &fiber) -> bool {
          const bool remove = fiber->type()->has_path("fiber") && !fiber->running;
          if (remove) {
            LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !yprocess!! destoyed\n", fiber->id()->toString().c_str());
          }
          return remove;
        });
        vTaskDelay(1); // feeds the watchdog for the task
      }
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto *thread = static_cast<Thread *>(vptr_thread);
      // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
      while (thread->running) {
        thread->loop();
        vTaskDelay(1); // feeds the watchdog for the task
      }
      // ESP_ERROR_CHECK(heap_trace_stop());
      // heap_trace_dump();

      Scheduler::singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->id()->equals(*thread->id());
        if (remove) {
          LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !y%process!! destoyed\n", proc->id()->toString().c_str());
        }
        return remove;
      });
      vTaskDelete(nullptr);
    }
  };
  ptr<Scheduler> scheduler() { return Options::singleton()->scheduler<Scheduler>(); }
} // namespace fhatos

#endif
#endif
