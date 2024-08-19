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
///
#include <process/x_scheduler.hpp>

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

    void feedLocalWatchdog() override {
      vTaskDelay(1); // feeds the watchdog for the task
    }

    virtual bool spawn(const Process_p &process) override {
      bool success = *this->processes_mutex_.write<bool>([this, process]() {
        process->setup();
        if (!process->running()) {
          LOG_PROCESS(ERROR, this, "!RUnable to spawn running %s: %s!!\n", ProcessTypes.toChars(process->ptype).c_str(),
                      process->id()->toString().c_str());
          return share(false);
        }
        // scheduler subscription listening for noobj "kill process" messages
        router()->route_subscription(share(Subscription{
            .source = *this->id(), .pattern = *process->id(), .onRecv = [process](const Message_p &message) {
              if (message->payload->isNoObj()) {
                process->stop();
              }
            }}));
        ////////////////////////////////
        bool success = false;
        switch (process->ptype) {
          case PType::THREAD: {
            const BaseType_t threadResult =
                xTaskCreatePinnedToCore(THREAD_FUNCTION, // Function that should be called
                                        process->id()->toString().c_str(), // Name of the task (for debugging)
                                        10000, // Stack size (bytes)
                                        process.get(), // Parameter to pass
                                        CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                        &static_cast<Thread *>(process.get())->handle, // Task handle
                                        tskNO_AFFINITY); // Processor core
            success = pdPASS == threadResult;
            if (success)
              this->processes_->insert({process->id(), process});
            break;
          }
          case PType::FIBER: {
            this->processes_->insert({process->id(), process});
            if (!FIBER_THREAD_HANDLE) {
              success &= pdPASS == xTaskCreatePinnedToCore(FIBER_FUNCTION, // Function that should be called
                                                           "fiber_bundle", // Name of the task (for debugging)
                                                           15000, // Stack size (bytes)
                                                           nullptr, // Parameter to pass
                                                           CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                                           &FIBER_THREAD_HANDLE, // Task handle
                                                           tskNO_AFFINITY); // Processor core
            }
            break;
          }
          case PType::COROUTINE: {
            success = true;
            LOG_PROCESS(INFO, this, "!b%s!! !ythreadless %s processs!! ignored\n", process->id()->toString().c_str(),
                        ProcessTypes.toChars(process->ptype).c_str());
            return share(success);
          }
        }
        if (success) {
          LOG_PROCESS(success ? INFO : ERROR, this, "!b%s!! !y%s!! spawned\n", process->id()->toString().c_str(),
                      ProcessTypes.toChars(process->ptype).c_str());
        }
        return share(success);
      });
      if (!success)
        router()->route_unsubscribe(this->id(), p_p(*process->id()));
      LOG(NONE,
          "\t!yFree memory\n"
          "\t  !b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR
          "][flash: " FOS_BYTES_MB_STR "]\n",
          FOS_BYTES_MB(ESP.getFreeSketchSpace()), FOS_BYTES_MB(ESP.getFreeHeap()), FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));
      return success;
    }

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {}

    TaskHandle_t FIBER_THREAD_HANDLE = nullptr;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *voidptr) {
      int counter = 1;
      while (counter > 0) {
        counter = 0;
        for (const auto &[id, proc]: *Scheduler::singleton()->processes_) {
          if (proc->ptype == PType::FIBER) {
            if (!proc->running())
              Scheduler::singleton()->kill(*proc->id());
            else {
              proc->loop();
              ++counter;
            }
          }
          vTaskDelay(1); // feeds the watchdog for the task
        }
      }
    }

      //////////////////////////////////////////////////////
      //////////////////////////////////////////////////////
      //////////////////////////////////////////////////////
      static void THREAD_FUNCTION(void *vptr_thread) {
        auto *thread = static_cast<Thread *>(vptr_thread);
        while (thread->running()) {
          thread->loop();
          vTaskDelay(1); // feeds the watchdog for the task
        }
        Scheduler::singleton()->_kill(*thread->id());
        vTaskDelete(nullptr);
      }
    };
    ptr<Scheduler> scheduler() { return Options::singleton()->scheduler<Scheduler>(); }
  } // namespace fhatos

#endif
#endif
