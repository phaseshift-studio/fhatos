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
#ifndef fhatos_process_hpp
#define fhatos_process_hpp

#include <atomic>
#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <thread>
#include <util/enums.hpp>
#include <util/ptr_helper.hpp>


#define FOS_ALREADY_STOPPED "!g[!b%s!g] !y%s!! already stopped\n"
#define FOS_ALREADY_SETUP "!g[!b%s!g] !y%s!! already setup\n"
#ifndef FOS_PROCESS_WDT_COUNTER
#define FOS_PROCESS_WDT_COUNTER 25
#endif

namespace fhatos {
  const ID_p THREAD_FURI = share<ID>(ID(REC_FURI->resolve("thread")));
  const ID_p FIBER_FURI = share<ID>(ID(REC_FURI->resolve("fiber")));
  const ID_p COROUTINE_FURI = share<ID>(ID(REC_FURI->resolve("coroutine")));

  class Process;
  using Process_p = ptr<Process>;
  static ptr<thread::id> scheduler_thread = nullptr;
  static atomic<Process *> this_process;

  enum class PType { THREAD, FIBER, COROUTINE };

  static const auto ProcessTypes =
      Enums<PType>({{PType::THREAD, "thread"}, {PType::FIBER, "fiber"}, {PType::COROUTINE, "coroutine"}});

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////


  class Process : public Obj {
  protected:
    std::atomic_bool running_ = false;
    std::atomic_int16_t wdt_timer_counter = 0;

  public:
    const PType ptype;

    explicit Process(const ID &id, const PType pType) :
      Obj(
          *Obj::to_rec({{vri(":setup"), Obj::to_bcode([this](const Obj_p &) {
                          this->setup();
                          return noobj();
                        }, "cpp:setup")},
                        {vri(":loop"), Obj::to_bcode([this](const Obj_p &) {
                          this->loop();
                          return noobj();
                        }, "cpp:loop")},
                        {vri(":stop"), Obj::to_bcode([this](const Obj_p &) {
                          this->stop();
                          return noobj();
                        }, "cpp:stop")},
                        {vri(":delay"), Obj::to_bcode([this](const Int_p &milliseconds) {
                          this->delay(milliseconds->int_value());
                          return noobj();
                        }, "cpp:delay")},
                        {vri(":yield"), Obj::to_bcode([this](const Obj_p &) {
                          this->yield();
                          return noobj();
                        }, "cpp:yield")}},
                       id_p(REC_FURI->extend(ProcessTypes.to_chars(pType))))), ptype(pType) {
      this->id_ = id_p(id);

    }

    ~Process() override = default;

    void feed_watchdog_via_counter() {
      if (this->wdt_timer_counter.fetch_add(1) >= FOS_PROCESS_WDT_COUNTER) {
        // LOG(INFO, "reset watchdog timer: %i >= %i\n", this->wdt_timer_counter.load(), FOS_PROCESS_WDT_COUNTER);
        FEED_WATCDOG();
        this->wdt_timer_counter.store(0);
      }
    }

    static Process *current_process() {
      return this_process.load();
    }

    virtual void setup() {
      this_process = this;
      if (this->running_.load()) {
        LOG(WARN, FOS_ALREADY_SETUP, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      this->running_.store(true);
    };

    virtual void loop() {
      if (!this->running_.load()) {
        throw fError("!g[!b%s!g] !y%s!! can't loop when stopped", this->id()->toString().c_str(),
                     ProcessTypes.to_chars(this->ptype).c_str());
      }
      this_process.store(this);
    };

    virtual void stop() {
      this_process = this;
      if (!this->running_.load()) {
        LOG(WARN, FOS_ALREADY_STOPPED, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      this->running_.store(false);
    };

    bool running() const { return this->running_.load(); }

    virtual void delay(const uint64_t) {
    }; // milliseconds

    virtual void yield() {
    };
  };
} // namespace fhatos

#endif