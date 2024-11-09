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

#define FOS_ALREADY_STOPPED "!g[!b%s!g] !yprocess!! already stopped\n"
#define FOS_ALREADY_SETUP "!g[!b%s!g] !yprocess!! already setup\n"
#ifndef FOS_PROCESS_WDT_COUNTER
#define FOS_PROCESS_WDT_COUNTER 25
#endif

namespace fhatos {
    const ID_p THREAD_FURI = share<ID>(ID(REC_FURI->resolve("thread")));
    const ID_p FIBER_FURI = share<ID>(ID(REC_FURI->resolve("fiber")));

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
    public:
        bool running = false;

    protected:
        int16_t wdt_timer_counter = 0;
        int32_t sleep_ = 0;
        bool yield_ = false;

    public:
        explicit Process(const Rec_p &setup_loop_stop) : Obj(rmap({
                                                                 {
                                                                     id_p(":delay"), Obj::to_bcode(
                                                                         [this](const Obj_p &milliseconds) {
                                                                             this->sleep_ = milliseconds->int_value();
                                                                             return noobj();
                                                                         }, StringHelper::cxx_f_metadata(
                                                                             __FILE__,__LINE__))
                                                                 },
                                                                 {
                                                                     id_p(":yield"), Obj::to_bcode(
                                                                         [this](const Obj_p &) {
                                                                             this->yield_ = true;
                                                                             return noobj();
                                                                         }, StringHelper::cxx_f_metadata(
                                                                             __FILE__,__LINE__))
                                                                 },
                                                                 {
                                                                     id_p(":halt"), Obj::to_bcode(
                                                                         [this](const Obj_p &) {
                                                                             this->stop();
                                                                             return noobj();
                                                                         }, StringHelper::cxx_f_metadata(
                                                                             __FILE__,__LINE__))
                                                                 }
                                                             }),
                                                             OType::REC, REC_FURI, setup_loop_stop->vid()) {
            this_process.store(this);
            this->rec_value()->insert(setup_loop_stop->rec_value()->begin(),setup_loop_stop->rec_value()->end());
            this->vid_ = setup_loop_stop->vid();
            this->tid_ = setup_loop_stop->tid();
        }

        ~Process() override = default;

        void feed_watchdog_via_counter() {
            if (++this->wdt_timer_counter >= FOS_PROCESS_WDT_COUNTER) {
                // LOG(INFO, "reset watchdog timer: %i >= %i\n", this->wdt_timer_counter.load(), FOS_PROCESS_WDT_COUNTER);
                FEED_WATCDOG();
                this->wdt_timer_counter = 0;
            }
        }

        static Process *current_process() {
            return this_process.load();
        }

        virtual void setup() {
            this_process = this;
            const BCode_p setup_bcode = ROUTER_READ(id_p(this->vid()->extend(":setup")));
            if (setup_bcode->is_bcode())
                Options::singleton()->processor<Obj>(noobj(), setup_bcode);
            else
                LOG_PROCESS(DEBUG, this, "setup !ybcode!! undefined\n");
            ////
            if (this->running) {
                LOG(WARN, FOS_ALREADY_SETUP, this->vid()->toString().c_str());
                return;
            }
            this->running = true;
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
            const BCode_p loop_bcode = ROUTER_READ(id_p(this->vid()->extend(":loop")));
            if (!loop_bcode->is_noobj()) {
                Obj_p result = Options::singleton()->processor<Obj>(noobj(), loop_bcode);
            } else
                throw fError("!b%s!! loop !ybcode!! undefined", this->vid()->toString().c_str());
        };

        virtual void stop() {
            this_process = this;
            const BCode_p stop_bcode = ROUTER_READ(id_p(this->vid()->extend(":stop")));
            if (stop_bcode->is_bcode())
                Options::singleton()->processor<Obj>(noobj(), stop_bcode);
            else
                LOG_PROCESS(DEBUG, this, "stop !ybcode!! undefined\n");
            if (!this->running) {
                LOG(WARN, FOS_ALREADY_STOPPED, this->vid()->toString().c_str());
                return;
            }
            this->running = false;
        };

        virtual void delay(const uint64_t milliseconds) {
            FEED_WATCDOG();
        }; // milliseconds

        virtual void yield() {
            FEED_WATCDOG();
        };
    };
} // namespace fhatos

#endif
