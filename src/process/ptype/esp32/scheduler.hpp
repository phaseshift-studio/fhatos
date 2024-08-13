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
    static Scheduler *singleton(const ID& id = ID("/scheduler/")) {
      static bool _setup = false;
      static Scheduler *scheduler = new Scheduler(id);
      if (!_setup) {
        scheduler->setup();
        _setup = true;
      }
      return scheduler;
    }

     void feedLocalWatchdog() override {
      vTaskDelay(1); // feeds the watchdog for the task
    }

    virtual bool spawn(Process *process) override {
      // TODO: have constructed processes NOT running or check is process ID already in scheduler
      process->setup();
      if (!process->running()) {
        LOG(ERROR, "!RUnable to spawn running %s: %s!!\n",
            ProcessTypes.toChars(process->type), process->id()->toString().c_str());
        return false;
      }
      this->subscribe(*process->id(), [this, process](const Message_p &message) {
        if (message->payload->isNoObj()) {
          this->unsubscribe(*process->id());
          process->stop();
        }
      });
      //////////////////////////////////////////////////
      ////// THREAD //////
      bool success = false;
      switch (process->type) {
        case PType::THREAD: {
          this->THREADS->push_back(static_cast<Thread *>(process));
          const BaseType_t threadResult = xTaskCreatePinnedToCore(
            THREAD_FUNCTION, // Function that should be called
            process->id()->toString().c_str(), // Name of the task (for debugging)
            10000, // Stack size (bytes)
            process, // Parameter to pass
            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
            &static_cast<Thread *>(process)->handle, // Task handle
            tskNO_AFFINITY); // Processor core
          success = pdPASS == threadResult;
          break;
        }
        case PType::FIBER: {
          success = this->FIBERS->push_back(static_cast<Fiber *>(process));
          LOG(INFO, "Fiber bundle count: %i\n", this->FIBERS->size());
          if (!FIBER_THREAD_HANDLE) {
            success &= pdPASS == xTaskCreatePinnedToCore(
              FIBER_FUNCTION, // Function that should be called
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
          success = this->COROUTINES->push_back(static_cast<Coroutine *>(process));
          break;
        }
        case PType::KERNEL: {
          success = this->KERNELS->push_back(static_cast<XKernel *>(process));
          break;
        }
        default: {
          LOG(ERROR, "!m%s!! has an unknown process type: !r%i!!\n",
              process->id()->toString().c_str(), process->type);
          return false;
        }
      }
      LOG_SPAWN(success,process);
      LOG(NONE,
          "\t!yFree memory\n"
          "\t  !b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR "][flash: "
          FOS_BYTES_MB_STR "]\n",
          FOS_BYTES_MB(ESP.getFreeSketchSpace()),
          FOS_BYTES_MB(ESP.getFreeHeap()),
          FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));
      return success;
    }

  private:
  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {
    }

    TaskHandle_t FIBER_THREAD_HANDLE = nullptr;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *voidptr) {
      auto *fibers = Scheduler::singleton()->FIBERS;
      int counter = 0;
      while (!fibers->empty()) {
        Option<Fiber *> fiber = fibers->get(counter);
        if (fiber.has_value()) {
          if (!fiber.value()->running())
            Scheduler::singleton()->kill(*fiber.value()->id());
          else
            fiber.value()->loop();
          counter = (counter + 1) % fibers->size();
        } else {
          counter = 0;
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
      auto *thread = static_cast<Thread *>(vptr_thread);
      while (thread->running()) {
        thread->loop();
        vTaskDelay(1); // feeds the watchdog for the task
      }
      Scheduler::singleton()->_kill(*thread->id());
      vTaskDelete(nullptr);
    }
  };
} // namespace fhatos

#endif
#endif
