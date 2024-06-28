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

#if defined(NATIVE)
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include <fhatos.hpp>
///
#include <process/abstract_scheduler.hpp>


namespace fhatos {
  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class Scheduler final : public AbstractScheduler<ROUTER> {
  public:
    static Scheduler *singleton() {
      static Scheduler scheduler = Scheduler();
      return &scheduler;
    }

    bool spawn(Process *process) override {
      // TODO: have constructed processes NOT running or check is process ID already in scheduler
      process->setup();
      if (!process->running()) {
        LOG(ERROR, "!RUnable to spawn running %s: %s!!\n", P_TYPE_STR(process->type), process->id()->toString().c_str());
        return false;
      }
      //////////////////////////////////////////////////
      ////// THREAD //////
      bool success = false;
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
          success = this->KERNELS->push_back(dynamic_cast<KernelProcess *>(process));
          break;
        }
        default: {
          LOG(ERROR, "!m%s!! has an unknown process type: !r%i!!\n", process->id()->toString().c_str(), process->type);
          return false;
        }
      }
      LOG(success ? INFO : ERROR, "!M%s!! %s spawned\n", process->id()->toString().c_str(), P_TYPE_STR(process->type));
      /*LOG(NONE,
          "\t!yFree memory\n"
          "\t  !b[inst:" FOS_BYTES_MB_STR "][heap: " FOS_BYTES_MB_STR "][psram: " FOS_BYTES_MB_STR "][flash: "
          FOS_BYTES_MB_STR "]\n",
          FOS_BYTES_MB(ESP.getFreeSketchSpace()),
          FOS_BYTES_MB(ESP.getFreeHeap()),
          FOS_BYTES_MB(ESP.getFreePsram()),
          FOS_BYTES_MB(ESP.getFlashChipSize()));*/
      return success;
    }

  private:
    explicit Scheduler(const ID_p &id = share(ROUTER::mintID("scheduler", "kernel"))) : AbstractScheduler<ROUTER>(id) {
    }

    std::thread *FIBER_THREAD_HANDLE = nullptr;
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
      Scheduler<>::singleton()->destroy(*thread->id());
    }
  };
} // namespace fhatos
#endif
#endif
