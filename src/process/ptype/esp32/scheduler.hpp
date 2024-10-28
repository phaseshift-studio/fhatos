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
//#include <esp_heap_trace.h>
#include <process/x_scheduler.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(fiber.hpp)

#ifndef FOS_ESP_THREAD_STACK_SIZE
#define FOS_ESP_THREAD_STACK_SIZE 25000
#endif
#ifndef FOS_ESP_FIBER_STACK_SIZE
#define FOS_ESP_FIBER_STACK_SIZE 20000
#endif


//#ifdef CONFIG_HEAP_TRACING_DEST
//#define NUM_RECORDS 100
//static heap_trace_record_t* trace_record = new heap_trace_record_t[NUM_RECORDS]; // This buffer must be in internal RAM
//#endif

namespace fhatos {
  class Scheduler final : public XScheduler {
  public:
    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool _setup = false;
      static ptr<Scheduler> scheduler = ptr<Scheduler>(new Scheduler(id));
      if (!_setup) {
        scheduler->setup();
        _setup = true;
      }
      return scheduler;
    }

    void feed_local_watchdog() override {
      vTaskDelay(1); // feeds the watchdog for the task
    }

    virtual bool spawn(const Process_p &process) override {
      process->setup();
      if (!process->running()) {
        LOG_SCHEDULER(ERROR, "!RUnable to spawn running %s: %s!!\n", ProcessTypes.to_chars(process->ptype).c_str(),
                      process->id()->toString().c_str());
        return false;
      }
      // scheduler subscription listening for noobj "kill process" messages
      /* router()->route_subscription(subscription_p(*this->id(),*process->id(), QoS::_1,
         Insts::to_bcode([process](const Message_p &message) { if (message->payload->is_noobj()) { process->stop();
             }
           })));*/
      ////////////////////////////////
      bool success = false;
      uint16_t stack_size = process->ptype == PType::FIBER
                                ? FOS_ESP_FIBER_STACK_SIZE
                                : (process->ptype == PType::THREAD ? FOS_ESP_THREAD_STACK_SIZE : 0);
      switch (process->ptype) {
        case PType::THREAD: {
          const BaseType_t threadResult =
              xTaskCreatePinnedToCore(THREAD_FUNCTION, // Function that should be called
                                      process->id()->toString().c_str(), // Name of the task (for debugging)
                                      stack_size, // Stack size (bytes)
                                      process.get(), // Parameter to pass
                                      CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                      &static_cast<Thread *>(process.get())->handle, // Task handle
                                      tskNO_AFFINITY); // Processor core
          success = pdPASS == threadResult;
          break;
        }
        case PType::FIBER: {
          success = true;
          if (!FIBER_THREAD_HANDLE) {
            success &= pdPASS == xTaskCreatePinnedToCore(FIBER_FUNCTION, // Function that should be called
                                                         "fiber_bundle", // Name of the task (for debugging)
                                                         stack_size, // Stack size (bytes)
                                                         nullptr, // Parameter to pass
                                                         CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                                         &FIBER_THREAD_HANDLE, // Task handle
                                                         tskNO_AFFINITY); // Processor core
          }
          break;
        }
        case PType::COROUTINE: {
          success = true;
          break;
        }
      }
      if (success) {
        this->processes_->push_back(process);
        LOG_SCHEDULER(success ? INFO : ERROR, "!b%s!! !y%s!! spawned (w/ %i bytes stack)\n",
                      process->id()->toString().c_str(), ProcessTypes.to_chars(process->ptype).c_str(), stack_size);
      } else
        router()->route_unsubscribe(this->id(), p_p(*process->id()));
      return success;
    }

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {
      //ESP_ERROR_CHECK(heap_trace_init_standalone(trace_record, NUM_RECORDS));
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
          if (proc->ptype == PType::FIBER)
            fibers->push_back(proc);
        });
        for (const Process_p &fiber: *fibers) {
          if (fiber->running())
            fiber->loop();
          counter++;
        }
        Scheduler::singleton()->processes_->remove_if([](const Process_p &fiber) -> bool {
          const bool remove = fiber->ptype == PType::FIBER && !fiber->running();
          if (remove)
            LOG_DESTROY(true, fiber, Scheduler::singleton());
          return remove;
        });
        vTaskDelay(1); // feeds the watchdog for the task
      }
    }


    /* static void FIBER_FUNCTION(void *voidptr) {
       int counter = 1;
       while (counter > 0) {
         counter = 0;
         for (const auto &[id, proc]: *Scheduler::singleton()->processes_) {
           if (proc->ptype == PType::FIBER) {
             if (!proc->running())
               Scheduler::singleton()->stop(*proc->id());
             else {
               proc->loop();
               ++counter;
             }
           }
           vTaskDelay(1); // feeds the watchdog for the task
         }
       }
     }*/

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto *thread = static_cast<Thread *>(vptr_thread);
      //ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
      while (thread->running()) {
        thread->loop();
        vTaskDelay(1); // feeds the watchdog for the task
      }
      //ESP_ERROR_CHECK(heap_trace_stop());
      //heap_trace_dump();

      Scheduler::singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->id()->equals(*thread->id());
        if (remove)
          LOG_DESTROY(true, proc, Scheduler::singleton());
        return remove;
      });
      vTaskDelete(nullptr);
    }
  };
  ptr<Scheduler> scheduler() { return Options::singleton()->scheduler<Scheduler>(); }
} // namespace fhatos

#endif
#endif
