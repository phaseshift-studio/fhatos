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
#include "language/processor/processor.hpp"
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

namespace fhatos {
  class Sys;
  static atomic_int FIBER_COUNT;

  class Scheduler final : public XScheduler {
    friend Sys;

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")):
      XScheduler(id) {
      this->Obj::rec_set(vri(":spawn"), to_bcode([this](const Obj_p &obj) {
        if (!obj->vid())
          throw fError("value id required to spawn %s", obj->toString().c_str());
        if (obj->tid()->has_path("thread"))
          return dool(this->spawn(make_shared<Thread>(obj)));
        if (obj->tid()->has_path("fiber"))
          return dool(this->spawn(make_shared<Fiber>(obj)));
        throw fError("unknown process type: %s\n", obj->tid()->toString().c_str());
      }, StringHelper::cxx_f_metadata(__FILE__,__LINE__)));
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
        scheduler_thread = make_shared<thread::id>(std::this_thread::get_id());
        setup = true;
      }
      return scheduler_p;
    }

    void feed_local_watchdog() override {
    }

    bool spawn(const Process_p &process) override {
      if (this->count(*process->vid())) {
        LOG_KERNEL_OBJ(ERROR, this, FURI_WRAP "  !yprocess!! already running\n", process->vid()->toString().c_str());
        return false;
      }
      process->setup();
      if (!process->running) {
        LOG_KERNEL_OBJ(ERROR, this, FURI_WRAP " !yprocess!! failed to spawn\n", process->vid()->toString().c_str());
        return false;
      }
      ////////////////////////////////
      if (process->tid()->has_path("thread"))
        static_cast<Thread *>(process.get())->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, process.get());
      else if (process->tid()->has_path("fiber")) {
        if (!FIBER_THREAD_HANDLE) {
          FIBER_THREAD_HANDLE = new std::thread(&Scheduler::FIBER_FUNCTION, nullptr);
        }
        static_cast<Fiber *>(process.get())->xthread = FIBER_THREAD_HANDLE;
        static_cast<Fiber *>(process.get())->FIBER_COUNT = &FIBER_COUNT;
      } else {
        process->running = false;
        LOG_KERNEL_OBJ(ERROR, this, FURI_WRAP " !yprocess!! failed to spawn\n", process->vid()->toString().c_str());
        return false;
      }
      this->processes_->push_back(process);
      process->save();
      LOG_KERNEL_OBJ(INFO, this, FURI_WRAP " !yprocess!! spawned\n", process->vid()->toString().c_str());
      this->save();
      return true;
    }

    static void *import() {
      XScheduler::base_import(Scheduler::singleton());
      return nullptr;
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
          if (process->tid()->has_path("fiber") && process->running)
            fibers->push_back(process);
        });
        FIBER_COUNT = 0;
        for (const Process_p &fiber: *fibers) {
          if (fiber->running) {
            fiber->loop();
            ++FIBER_COUNT;
          }
        }
        singleton()->processes_->remove_if([](const Process_p &fiber) -> bool {
          const bool remove = fiber->tid()->has_path("fiber") && !fiber->running;
          if (remove) {
            LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !yprocess!! destoyed\n",
                                 fiber->vid()->toString().c_str());
          }
          return remove;
        });
        singleton()->save();
        delete fibers;
      }
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto thread = static_cast<Thread *>(vptr_thread);
      try {
        while (thread->running) {
          thread->loop();
        }
      } catch (const fError &error) {
        thread->stop();
        LOG_PROCESS(ERROR, thread, "processor loop error: %s\n", error.what());
      }
      singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->vid()->equals(*thread->vid()) || !proc->running;
        if (remove) {
          LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !yprocess!! destoyed\n", proc->vid()->toString().c_str());
        }
        return remove;
      });
      singleton()->save();
    }
  };

  inline ptr<Scheduler> scheduler() { return Scheduler::singleton(); }

} // namespace fhatos
#endif
#endif