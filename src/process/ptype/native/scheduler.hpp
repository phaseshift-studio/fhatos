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

#include "../../../fhatos.hpp"
#include "../../../lang/processor/processor.hpp"
#include "../../../process/base_scheduler.hpp"
#include "../../../process/process.hpp"
#include "thread.hpp"

namespace fhatos {
  class Sys;
  static atomic_int FIBER_COUNT;

  class Scheduler final : public BaseScheduler {

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")): BaseScheduler(id) {
    }

  public:
    ~Scheduler() override = default;

    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool setup = false;
      static auto scheduler_p = ptr<Scheduler>(new Scheduler(id));
      if(!setup) {
        scheduler_thread = make_shared<thread::id>(std::this_thread::get_id());
        setup = true;
      }
      return scheduler_p;
    }

    void feed_local_watchdog() override {
    }

    bool spawn(const Process_p &process) override {
      if(!process->vid_)
        throw fError("value id required to spawn %s", process->toString().c_str());
      if(this->count(*process->vid_)) {
        LOG_KERNEL_OBJ(ERROR, this, FURI_WRAP "  !yprocess!! already running\n", process->vid_->toString().c_str());
        return false;
      }
      process->setup();
      if(!process->running) {
        LOG_KERNEL_OBJ(ERROR, this, FURI_WRAP " !yprocess!! failed to spawn\n", process->vid_->toString().c_str());
        return false;
      }
      ////////////////////////////////
      static_cast<Thread *>(process.get())->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, process.get());
      this->processes_->push_back(process);
      process->save();
      LOG_KERNEL_OBJ(INFO, this, FURI_WRAP " !yprocess!! spawned\n", process->vid_->toString().c_str());
      this->save();
      return true;
    }

    static void *import() {
      BaseScheduler::base_import(Scheduler::singleton());
      return nullptr;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    std::thread *FIBER_THREAD_HANDLE = nullptr;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void THREAD_FUNCTION(void *vptr_thread) {
      auto thread = static_cast<Thread *>(vptr_thread);
      try {
        while(thread->running) {
          thread->loop(); // the thread loop
        }
      } catch(const fError &error) {
        thread->stop(); // exception shuts down the thread
        LOG_PROCESS(ERROR, thread, "processor loop error: %s\n", error.what());
      }
      // when a thread dies, the entire thread pool is searched for stale threads and removes them along with this thread
      singleton()->processes_->remove_if([thread](const Process_p &proc) {
        const bool remove = proc->vid_->equals(*thread->vid_) || !proc->running;
        if(remove) {
          LOG_SCHEDULER_STATIC(INFO, FURI_WRAP " !yprocess!! destroyed\n", proc->vid_->toString().c_str());
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
