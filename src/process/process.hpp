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

#include <thread>
#include <atomic>
#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../lang/obj.hpp"

#define FOS_ALREADY_STOPPED "!g[!b%s!g] !yprocess!! already stopped\n"
#define FOS_ALREADY_SETUP "!g[!b%s!g] !yprocess!! already setup\n"
#ifndef FOS_PROCESS_WDT_COUNTER
#define FOS_PROCESS_WDT_COUNTER 25
#endif

namespace fhatos {
  const ID_p PROCESS_FURI = make_shared<ID>("/sys/scheduler/lib/process");
  const ID_p THREAD_FURI = make_shared<ID>("/sys/scheduler/lib/thread");
  const ID_p FIBER_FURI = make_shared<ID>("/sys/scheduler/lib/fiber");

  class Process;
  using Process_p = ptr<Process>;
  static ptr<thread::id> scheduler_thread = nullptr;
  static atomic<Process *> this_process;

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////


  class Process : public Rec {
  public:
    bool running = false;

  public:
    int16_t wdt_timer_counter = 0;
    int32_t sleep_ = 0;
    bool yield_ = false;
    bool stop_ = false;

  public:
    explicit Process() : Rec({}, OType::REC, id_p("/sys/scheduler/type")) {
      this->vid_ = id_p("/sys/scheduler");
    }

    explicit Process(const ID_p &value_id, const Rec_p &setup_loop_stop) : Rec(setup_loop_stop->rec_value(),
                                                                               OType::REC, setup_loop_stop->tid_, value_id) {
    };
    /*
      Obj::to_inst(
        , {x(0, Obj::to_bcode())}, INST_FURI, make_shared<ID>(string(StringHelper::cxx_f_metadata(
          "/sys/scheduler/lib", "process",__LINE__))))
    },
    {
      id_p(":yield"), Obj::to_inst(
        [this](const Obj_p &, const InstArgs &) {
          Process::current_process()->yield_ = true;
          return noobj();
        }, NO_ARGS, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(
          "/sys/scheduler/lib", "process",__LINE__)))
    },
    {
      id_p(":stop"), Obj::to_inst(
        [this](const Obj_p &, const InstArgs &) {
          Process::current_process()->stop_ = true;
          return noobj();
        }, NO_ARGS, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(
          "/sys/scheduler/lib", "process",__LINE__)))
    }*/

    /* ObjHelper::InstTypeBuilder::build("/type/process/inst/delay")
         ->type_args(x(0, "milliseconds", Obj::to_bcode()))
         ->inst_f([this](const Obj_p &, const InstArgs &args) {
           this->sleep_ = args.at(0)->int_value();
           return _noobj_;
         })->create(id_p(SCHEDULER_ID->extend("lib/process/inst/delay")));
     ObjHelper::InstTypeBuilder::build(*INST_FURI)
         ->inst_f([this](const Obj_p &, const InstArgs &) {
           this->yield_ = true;
           return _noobj_;
         })->create(id_p("/sys/lib/scheduler/process/:yield"));
     ObjHelper::InstTypeBuilder::build(*INST_FURI)
         ->inst_f([this](const Obj_p &, const InstArgs &) {
           this->stop();
           return _noobj_;
         })->create(id_p("/sys/lib/scheduler/process/:stop"));*/

    ~Process() override = default;

    void feed_watchdog_via_counter() {
      if(++this->wdt_timer_counter >= FOS_PROCESS_WDT_COUNTER) {
        // LOG(INFO, "reset watchdog timer: %i >= %i\n", this->wdt_timer_counter.load(), FOS_PROCESS_WDT_COUNTER);
        FEED_WATCDOG();
        this->wdt_timer_counter = 0;
      }
    }

    static Process *current_process() {
      if(this_process)
        return this_process.load();
      else {
        static auto proc = new Process();
        proc->vid_ = id_p("/sys/scheduler");
        return proc;
      }
    }

    virtual void setup() {
      this_process = this;
      /*ROUTER_SUBSCRIBE(Subscription::create(*this->vid_, this->vid_->extend(":loop"),
                                            InstBuilder::build(this->vid_->extend(":loop"))
                                            ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
                                              this->rec_set(":loop", lhs);
                                              return noobj();
                                            })
                                            ->create()));*/
      const BCode_p setup_bcode = ROUTER_READ(id_p(this->vid_->extend(":setup")));
      if(setup_bcode->is_noobj())
        LOG_PROCESS(DEBUG, this, "setup !ybcode!! undefined\n");
      else
        BCODE_PROCESSOR(setup_bcode);
      ////
      if(this->running) {
        LOG(WARN, FOS_ALREADY_SETUP, this->vid_->toString().c_str());
        return;
      }
      this->running = true;
    };

    virtual void loop() {
      this_process = this;
      if(this->stop_) {
        this->stop();
        return;
      }
      if(this->sleep_ > 0) {
        this->delay(sleep_);
        this->sleep_ = 0;
      }
      if(this->yield_) {
        this->yield();
        this->yield_ = false;
      }
      const BCode_p loop_bcode = ROUTER_READ(this->vid_)->rec_get("/:loop");
      if(loop_bcode->is_noobj())
        throw fError("!b%s !ybcode!! undefined: %s", this->vid_->append("/:loop").toString().c_str(),
                     this->toString().c_str());
      Obj_p result = BCODE_PROCESSOR(loop_bcode);
    };

    virtual void stop() {
      this_process = this;
      if(!this->running) {
        LOG(WARN, FOS_ALREADY_STOPPED, this->vid_->toString().c_str());
        return;
      }
      const BCode_p stop_bcode = ROUTER_READ(id_p(this->vid_->extend(":stop")));
      if(stop_bcode->is_noobj())
        LOG_PROCESS(DEBUG, this, "stop !ybcode!! undefined\n");
      else
        BCODE_PROCESSOR(stop_bcode);

      this->running = false;
    };

    virtual void delay(const uint64_t) {
      FEED_WATCDOG();
    }; // milliseconds

    virtual void yield() {
      FEED_WATCDOG();
    };
  };
} // namespace fhatos

#endif
