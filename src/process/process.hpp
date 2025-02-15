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
#include "../util/obj_helper.hpp"

#define FOS_ALREADY_STOPPED "!g[!b%s!g] !yprocess!! already stopped\n"
#define FOS_ALREADY_SETUP "!g[!b%s!g] !yprocess!! already setup\n"
#ifndef FOS_PROCESS_WDT_COUNTER
#define FOS_PROCESS_WDT_COUNTER 25
#endif

namespace fhatos {
  static const auto THREAD_FURI = id_p("/sys/lib/thread");

  class Process;
  using Process_p = ptr<Process>;
  static ptr<thread::id> scheduler_thread = nullptr;
  static atomic<Process *> this_process;

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////


  class Process : public Obj {

  public:
    bool running = false;
    int16_t wdt_timer_counter = 0;
    int32_t sleep_ = 0;
    bool yield_ = false;
    bool stop_ = false;
    BCode_p loop_code_ = nullptr;
    //Obj_p last_result_ = Obj::to_noobj();


    explicit Process() : Rec({}, OType::REC, REC_FURI) { // hack for scheduler "process"
      this->vid = id_p("/sys/scheduler");
    }

    explicit Process(const Obj_p &obj) : Obj(obj->value_, obj->otype, obj->tid) {
      this->vid = obj->vid;
    };

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
        proc->vid = id_p("/sys/scheduler");
        return proc;
      }
    }

    virtual void setup() {
      this_process = this;
      /*ROUTER_SUBSCRIBE(Subscription::create(*this->vid, this->vid->extend(":loop"),
                                            InstBuilder::build(this->vid->extend(":loop"))
                                            ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
                                              this->rec_set(":loop", lhs);
                                              return noobj();
                                            })
                                            ->create()));*/
      const ID_p stop_id = id_p(this->vid->extend(":stop"));
      const ID_p delay_id = id_p(this->vid->extend(":delay"));
      const ID_p yield_id = id_p(this->vid->extend(":yield"));
      //if(ROUTER_READ(stop_id)->is_noobj())
      InstBuilder::build(stop_id)
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([this](const Obj_p &lhs, const InstArgs &) {
            this->stop_ = true;
            return lhs;
          })->create(stop_id);
      InstBuilder::build(delay_id)
          ->type_args(x(0, Obj::to_bcode()))
          ->domain_range(
            OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
            this->sleep_ = args->arg(0)->int_value();
            return lhs;
          })->create(delay_id);
      InstBuilder::build(yield_id)
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([this](const Obj_p &lhs, const InstArgs &) {
            this->yield_ = true;
            return lhs;
          })->create(yield_id);
      this->load();

      if(const BCode_p setup_bcode = ROUTER_READ(this->vid->extend(":setup")); setup_bcode->is_noobj())
        LOG_PROCESS(DEBUG, this, "setup !ybcode!! undefined\n");
      else
        BCODE_PROCESSOR(setup_bcode);
      ////
      if(this->running) {
        LOG(WARN, FOS_ALREADY_SETUP, this->vid->toString().c_str());
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
      if(!this->loop_code_)
        this->loop_code_ = ROUTER_READ(this->vid->extend(":loop"));
      if(!this->loop_code_ || this->loop_code_->is_noobj())
        throw fError("!b%s !ybcode!! undefined: %s", this->vid->extend(":loop").toString().c_str(),
                     this->toString().c_str());
      /*this->last_result_ = */
      // TODO: make a BCODE_PROCESSOR that takes both bcode and starts w/o having to inline starts
      BCODE_PROCESSOR(this->loop_code_);
    };

    virtual void stop() {
      this_process = this;
      if(!this->running) {
        LOG(WARN, FOS_ALREADY_STOPPED, this->vid->toString().c_str());
        return;
      }
      if(const BCode_p stop_bcode = ROUTER_READ(this->vid->extend(":stop")); stop_bcode->is_noobj())
        LOG_PROCESS(DEBUG, this, "stop !ybcode!! undefined\n");
      else
        BCODE_PROCESSOR(stop_bcode);
      this->running = false;
      ROUTER_WRITE(this->vid->extend(":stop"), Obj::to_noobj(), true);
      ROUTER_WRITE(this->vid->extend(":delay"), Obj::to_noobj(), true);
      ROUTER_WRITE(this->vid->extend(":yield"), Obj::to_noobj(), true);
      this->load();
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
