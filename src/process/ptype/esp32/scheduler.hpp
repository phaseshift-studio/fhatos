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

#include "../../../fhatos.hpp"
#include "../../../lang/processor/processor.hpp"
#include "../../../process/base_scheduler.hpp"
#include "../../../process/process.hpp"
#include "thread.hpp"


#ifndef FOS_ESP_THREAD_STACK_SIZE
#define FOS_ESP_THREAD_STACK_SIZE 16192
#endif


// #ifdef CONFIG_HEAP_TRACING_DEST
// #define NUM_RECORDS 100
//  static heap_trace_record_t* trace_record = new heap_trace_record_t[NUM_RECORDS]; // This buffer must be in internal
//  RAM #endif

namespace fhatos {
  class Scheduler final : public BaseScheduler {
  public:
    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool setup_ = false;
      static ptr<Scheduler> scheduler = ptr<Scheduler>(new Scheduler(id));
      if (!setup_) {
        // scheduler_thread = make_shared<thread::id>(this_thread::get_id());
        setup_ = true;
      }
      return scheduler;
    }

    static void *import() {
      BaseScheduler::base_import(Scheduler::singleton());
      return nullptr;
    }

    void feed_local_watchdog() override {
      vTaskDelay(1); // feeds the watchdog for the task
    }

    Process_p raw_spawn(const Process_p &process) override {
      int stack_size;
      if(process->is_rec()) {
        stack_size = process->rec_get("stack_size") // check provided obj
        ->or_else(process->rec_get("+/stack_size")->none_one()) // check one depth more (e.g. config/stack_size)
        ->or_else(this->rec_get("config/def_stack_size") // check default setting in scheduler
        ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE))) // use default environmental variable
        ->int_value();
      } else {
        stack_size = this->rec_get("config/def_stack_size") // check default setting in scheduler
        ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE)) // use default environmental variable
        ->int_value();
      }
      const BaseType_t threadResult = xTaskCreatePinnedToCore(THREAD_FUNCTION, // Function that should be called
                                               process->vid->toString().c_str(), // Name of the task (for debugging)
                                               stack_size, // Stack size (bytes)
                                               process.get(), // Parameter to pass
                                               CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                               static_cast<Thread *>(process.get())->xthread, // Task handle
                                               tskNO_AFFINITY); // Processor core
      return (pdPASS == threadResult) ? process : nullptr;
    }


  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : BaseScheduler(id) {
      // ESP_ERROR_CHECK(heap_trace_init_standalone(trace_record, NUM_RECORDS));
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto *thread = static_cast<Thread *>(vptr_thread);
      // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
      try {
        while (thread->running) {
          thread->loop();
          FEED_WATCDOG(); // feeds the watchdog for the task
        }
      } catch (fError error) {
        thread->stop();
        LOG_PROCESS(ERROR, thread, "pre-processor error: %s\n", error.what());
      }
      // ESP_ERROR_CHECK(heap_trace_stop());
      // heap_trace_dump();

      Scheduler::singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->vid->equals(*thread->vid);
        if (remove) {
          Router::singleton()->unsubscribe(*singleton()->vid,*proc->vid);
          LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !y%process!! destroyed\n", proc->vid->toString().c_str());
        }
        return remove;
      });
      vTaskDelete(nullptr);
    }
  };
  inline ptr<Scheduler> scheduler() { return Scheduler::singleton(); }
} // namespace fhatos

#endif
#endif
