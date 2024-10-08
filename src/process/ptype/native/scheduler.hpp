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
#include <util/ptr_helper.hpp>
#include <process/process.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {
  static atomic_int FIBER_COUNT;

  class Scheduler final : public XScheduler {
  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")): XScheduler(id) {
    }

  public:
    ~Scheduler() override {
      if (FIBER_THREAD_HANDLE) {
        if (FIBER_THREAD_HANDLE->joinable()) {
          this->FIBER_THREAD_HANDLE->detach();
        }
        delete this->FIBER_THREAD_HANDLE;
      }
    }

    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool setup = false;
      static auto scheduler_p = ptr<Scheduler>(new Scheduler(id));
      if (!setup) {
        scheduler_thread = make_shared<thread::id>(this_thread::get_id());
        scheduler_p->setup();
        setup = true;
      }
      return scheduler_p;
    }

    void feed_local_watchdog() override {
    }

    bool spawn(const Process_p &process) override {
      process->setup();
      if (!process->running()) {
        LOG_SCHEDULER(ERROR, "!RUnable to spawn running %s: %s!!\n", ProcessTypes.to_chars(process->ptype).c_str(),
                      process->id()->toString().c_str());
        return false;
      }
      ////////////////////////////////
      switch (process->ptype) {
        case PType::THREAD: {
          static_cast<Thread *>(process.get())->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, process.get());
          break;
        }
        case PType::FIBER: {
          if (!FIBER_THREAD_HANDLE) {
            FIBER_THREAD_HANDLE = new std::thread(&Scheduler::FIBER_FUNCTION, nullptr);
          }
          static_cast<Fiber *>(process.get())->xthread = FIBER_THREAD_HANDLE;
          static_cast<Fiber *>(process.get())->FIBER_COUNT = &FIBER_COUNT;
          break;
        }
        case PType::COROUTINE: {
          break;
        }
      }
      this->processes_->push_back(process);
      LOG_SCHEDULER(INFO, "!b%s!! !y%s!! spawned\n", process->id()->toString().c_str(),
                    ProcessTypes.to_chars(process->ptype).c_str());
      return true;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    std::thread *FIBER_THREAD_HANDLE = nullptr;

    static void FIBER_FUNCTION(void *) {
      FIBER_COUNT = 1;
      while (FIBER_COUNT > 0) {
        auto *fibers = new List<Process_p>();
        singleton()->processes_->forEach([fibers](const Process_p &process) {
          if (process->ptype == PType::FIBER && process->running())
            fibers->push_back(process);
        });
        FIBER_COUNT = 0;
        for (const Process_p &fiber: *fibers) {
          if (fiber->running()) {
            fiber->loop();
            ++FIBER_COUNT;
          }
        }
        singleton()->processes_->remove_if([](const Process_p &fiber) -> bool {
          const bool remove = fiber->ptype == PType::FIBER && !fiber->running();
          if (remove) LOG_DESTROY(true, fiber, Scheduler::singleton());
          return remove;
        });
        delete fibers;
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
      singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->id()->equals(*thread->id()) || !proc->running();
        if (remove) LOG_DESTROY(true, proc, Scheduler::singleton());
        return remove;
      });
    }
  };

  inline ptr<Scheduler> scheduler() { return Options::singleton()->scheduler<Scheduler>(); }
} // namespace fhatos
#endif
#endif
