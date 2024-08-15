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

namespace fhatos {
  class Scheduler final : public XScheduler {

  private:
    explicit Scheduler(const ID &id = ID("/scheduler/")) : XScheduler(id) {}

  public:
    static ptr<Scheduler> singleton(const ID &id = ID("/scheduler/")) {
      static bool _setup = false;
      static auto scheduler_p = ptr<Scheduler>(new Scheduler(id));
      if (!_setup) {
        scheduler_p->setup();
        _setup = true;
      }
      return scheduler_p;
    }

    bool spawn(const Process_p &process) override {
      bool success = *this->processes_mutex_.write<bool>([this, process]() {
        process->setup();
        if (!process->running()) {
          LOG_PROCESS(ERROR, this, "!RUnable to spawn running %s: %s!!\n", ProcessTypes.toChars(process->ptype).c_str(),
                      process->id()->toString().c_str());
          return share(false);
        }
        // scheduler subscription listening for noobj "kill process" messages
        router()->route_subscription(share(Subscription{
            .source = *this->id(), .pattern = *process->id(), .onRecv = [this, process](const Message_p &message) {
              if (message->payload->isNoObj()) {
                router()->route_unsubscribe(this->id(), p_p(*process->id()));
                this->_kill(*process->id());
              }
            }}));
        ////////////////////////////////
        bool success = false;
        this->processes_->insert({process->id(), process});
        switch (process->ptype) {
          case PType::THREAD: {
            ((Thread *) process.get())->xthread = new std::thread(&Scheduler::THREAD_FUNCTION, process.get());
            success = true;
            break;
          }
          case PType::FIBER: {
            // LOG(INFO, "Fiber bundle count: %i\n", this->FIBERS->size());
            if (!FIBER_THREAD_HANDLE) {
              FIBER_THREAD_HANDLE = new std::thread(&Scheduler::FIBER_FUNCTION, nullptr);
            }
            ((Fiber *) (process.get()))->xthread = FIBER_THREAD_HANDLE;
            success = true;
            break;
          }
          case PType::COROUTINE: {
            success = true;
            break;
          }
        }
        LOG_PROCESS(success ? INFO : ERROR, this, "!b%s!! !y%s!! spawned\n", process->id()->toString().c_str(),
                    ProcessTypes.toChars(process->ptype).c_str());
        return share(success);
      });
      if (!success)
        router()->route_unsubscribe(this->id(), p_p(*process->id()));
      return success;
    }

    std::thread *FIBER_THREAD_HANDLE = nullptr;
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    static void FIBER_FUNCTION(void *) {
      while (0 != Scheduler::singleton()->processes_mutex_.read<int>([]() {
        int counter = 0;
        for (const auto &[id, proc]: *Scheduler::singleton()->processes_) {
          if (proc->ptype == PType::FIBER) {
            ++counter;
            if (!proc->running())
              Scheduler::singleton()->kill(*proc->id());
            else
              proc->loop();
          }
        }
        return counter;
      })) {
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
      Scheduler::singleton()->_kill(*thread->id());
    }
  };
  ptr<Scheduler> scheduler() { return Options::singleton()->scheduler<Scheduler>(); }
} // namespace fhatos
#endif
#endif
