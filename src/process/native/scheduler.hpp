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
#if defined(NATIVE)
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include <fhatos.hpp>
///
#include <process/x_scheduler.hpp>

namespace fhatos {
  class Scheduler final : public XScheduler {

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {}

  public:
    static Scheduler *singleton(const ID &id = ID("/scheduler/")) {
      static bool _setup = false;
      static Scheduler scheduler = Scheduler(id);
      if (!_setup) {
        scheduler.setup();
        _setup = true;
      }
      return &scheduler;
    }

    bool spawn(XProcess *process) override {
      bool success = *RW_PROCESS_MUTEX.write<bool>([this, process]() {
        // TODO: have constructed processes NOT running or check is process ID already in scheduler
        process->setup();
        if (!process->running()) {
          LOG_TASK(ERROR, this, "!RUnable to spawn running %s: %s!!\n", P_TYPE_STR(process->type),
                   process->id()->toString().c_str());
          return share(false);
        }
        bool success = false;
        this->subscribe(*process->id(), [this, process](const Message_p &message) {
          if (message->payload->isNoObj()) {
            this->unsubscribe(*process->id());
            process->stop();
          }
        });
        ////////////////////////////////
        switch (process->type) {
          case THREAD: {
            this->THREADS->push_back(static_cast<Thread *>(process));
            dynamic_cast<Thread *>(process)->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, process);
            success = true;
            break;
          }
          case FIBER: {
            success = this->FIBERS->push_back(dynamic_cast<Fiber *>(process));
            LOG(INFO, "Fiber bundle count: %i\n", this->FIBERS->size());
            if (!FIBER_THREAD_HANDLE) {
              FIBER_THREAD_HANDLE = new std::thread(&Scheduler::FIBER_FUNCTION, nullptr);
              if (FIBER_THREAD_HANDLE)
                success = true;
            }
            dynamic_cast<Fiber *>(process)->xthread = FIBER_THREAD_HANDLE;
            break;
          }
          case COROUTINE: {
            success = this->COROUTINES->push_back(dynamic_cast<Coroutine *>(process));
            break;
          }
          case KERNEL: {
            success = this->KERNELS->push_back(dynamic_cast<XKernel *>(process));
            break;
          }
          default: {
            LOG_TASK(ERROR, this, "!b%s!! has an unknown process type: !r%i!!\n", process->id()->toString().c_str(),
                     process->type);
            return share(false);
          }
        }
        LOG_TASK(success ? INFO : ERROR, this, "!b%s!! !y%s!! spawned\n", process->id()->toString().c_str(),
                 P_TYPE_STR(process->type));
        return share(success);
      });
      if (!success)
        this->unsubscribe(*process->id());
      return success;
    }

    std::thread *FIBER_THREAD_HANDLE = nullptr;
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *) {
      auto *fibers = Scheduler::singleton()->FIBERS;
      int counter = 0;
      while (!fibers->empty()) {
        Option<Fiber *> fiber = fibers->get(counter);
        if (fiber.has_value()) {
          if (!fiber.value()->running())
            Scheduler::singleton()->destroy(*fiber.value()->id());
          else
            fiber.value()->loop();
          counter = (counter + 1) % fibers->size();
        } else {
          counter = 0;
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
      }
      Scheduler::singleton()->_destroy(*thread->id());
    }
  };
} // namespace fhatos
#endif
#endif
