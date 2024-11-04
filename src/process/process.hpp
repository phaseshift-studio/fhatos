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
    int32_t sleep_ = 0;
    bool yield_ = false;

  public:
    const PType ptype;

    explicit Process(const ID &id, const PType pType, const Rec_p &setup_loop_stop) :
      Obj(rmap({{id_p(":delay"), Obj::to_bcode([this](const Int_p &milliseconds) {
                  this->sleep_ = milliseconds->int_value();
                  return noobj();
                }, "cxx:delay")},
                {id_p(":yield"), Obj::to_bcode([this](const Obj_p &) {
                  this->yield_ = true;
                  return noobj();
                }, "cxx:yield")},
                {id_p(":halt"), Obj::to_bcode([this](const Obj_p &) {
                  this->stop();
                  return noobj();
                }, "cxx:yield")}}),
          REC_FURI, id_p(id)), ptype(pType) {
      this->rec_add(setup_loop_stop);
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
      const BCode_p setup_bcode = ROUTER_READ(id_p(this->id()->extend(":setup")));
      if (!setup_bcode->is_noobj())
        Options::singleton()->processor<Obj, BCode, Obj>(noobj(), setup_bcode);
      else
        LOG_PROCESS(INFO, this, "setup !ybcode!! undefined\n");
      ////
      if (this->running_.load()) {
        LOG(WARN, FOS_ALREADY_SETUP, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      this->running_ = true;
    };

    virtual void loop() {
      this_process = this;
      if (this->sleep_ > 0) {
        this->delay(sleep_);
        this->sleep_ = 0;
      }
      if (this->yield_) {
        this->yield();
        this->yield_ = false;
      }
      const BCode_p loop_bcode = ROUTER_READ(id_p(this->id()->extend(":loop")));
      if (!loop_bcode->is_noobj()) {
        Obj_p result = Options::singleton()->processor<Obj, BCode, Obj>(noobj(), loop_bcode);
      } else
        throw fError("!b%s!! loop !ybcode!! undefined", this->id()->toString().c_str());
    };

    virtual void stop() {
      this_process = this;
      const BCode_p stop_bcode = ROUTER_READ(id_p(this->id()->extend(":stop")));
      if (!stop_bcode->is_noobj())
        Options::singleton()->processor<Obj, BCode, Obj>(noobj(), stop_bcode);
      else
        LOG_PROCESS(INFO, this, "stop !ybcode!! undefined\n");
      if (!this->running_.load()) {
        LOG(WARN, FOS_ALREADY_STOPPED, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      this->running_ = false;
    };

    bool running() const { return this->running_.load(); }

    virtual void delay(const uint64_t) {
    }; // milliseconds

    virtual void yield() {
    };
  };
} // namespace fhatos

#endif