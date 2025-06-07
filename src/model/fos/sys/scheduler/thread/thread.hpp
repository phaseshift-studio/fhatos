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
#ifndef fhatos_thread_hpp
#define fhatos_thread_hpp
#include "../../../../../fhatos.hpp"
#include "../../../../../lang/mmadt/mmadt_obj.hpp"
#include "../../../../../lang/obj.hpp"
#include "../../../../fos/sys/router/memory/memory.hpp"
#include "../../typer/typer.hpp"

namespace fhatos {
  using namespace mmadt;
  class Thread;
  static auto this_thread = atomic<Thread *>(nullptr);
  static ID_p THREAD_FURI = id_p("/fos/sys/thread");

  class Thread {
  public:
    Consumer<Thread *> thread_function_;
    Any handler_;
    Obj_p thread_obj_;

    static Option<Thread *> current_thread() {
      if(this_thread.load())
        return {this_thread.load()};
      else {
        return {};
      }
    }

    template<typename HANDLER>
    HANDLER get_handler() const {
      return std::any_cast<HANDLER>(this->handler_);
    }

    static int core_current_thread() {
#ifdef ESP_PLATFORM
      return xPortGetCoreID();
#else
      return sched_getcpu();
#endif
    }

    static void delay(uint64_t milliseconds);

    static void yield();

    void halt() const;

    explicit Thread(
        const Obj_p &thread_obj, const Consumer<Thread *> &thread_function = [](const Thread *thread_ptr) {
          try {
            thread_ptr->thread_obj_->obj_set("halt", dool(false));
            const Obj_p thread_loop_obj =
                thread_ptr->thread_obj_->is_rec() && !thread_ptr->thread_obj_->rec_get("loop")->is_noobj()
                    ? thread_ptr->thread_obj_->obj_get("loop")
                    : Compiler(false).resolve_inst(thread_ptr->thread_obj_,
                                                   Obj::to_inst(Obj::to_inst_args(), id_p("loop"),
                                                                id_p(thread_ptr->thread_obj_->vid->extend("loop"))));
            const Inst_p thread_loop_inst = mmADT::delift(thread_loop_obj);
            LOG_WRITE(INFO, thread_ptr->thread_obj_.get(),
                      L("!g[!bfhatos!g] !ythread!! spawned: {} !m[!ystack size:!!{}!m]!!\n",
                        thread_loop_inst->toString(),
                        Memory::singleton()->get_stack_size(thread_ptr->thread_obj_, "config/stack_size", 65536)));
            while(!thread_ptr->thread_obj_->obj_get("halt")->or_else_<bool>(false)) {
              try {
                thread_loop_inst->apply(thread_ptr->thread_obj_);
                FEED_WATCHDOG();
              } catch(const fError &e) {
                LOG_WRITE(ERROR, thread_ptr->thread_obj_.get(), L("!rthread loop error!!: {}\n", e.what()));
                break;
              }
            }
            return Obj::to_noobj();
          } catch(const fError &e) {
            LOG_WRITE(ERROR, thread_ptr->thread_obj_.get(), L("!rthread construction error!!: {}\n", e.what()));
          }
          thread_ptr->thread_obj_->delete_model();
          thread_ptr->thread_obj_->obj_set("halt", dool(true));
          LOG_WRITE(INFO, thread_ptr->thread_obj_.get(), L("!ythread!! stopped\n"));
          thread_ptr->halt();
        });

    static ptr<Thread> create_state(const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); }

    static void register_module() {
      MODEL_CREATOR2->insert_or_assign(*THREAD_FURI,
                                       [](const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); });
      REGISTERED_MODULES->insert_or_assign(
          *THREAD_FURI,
          InstBuilder::build(Typer::singleton()->vid->add_component(*THREAD_FURI))
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec({{vri(THREAD_FURI), Obj::to_rec({{"loop", __()}, {"halt", __().else_(dool(true))}})},
                                    {vri(THREAD_FURI->add_component("spawn")),
                                     InstBuilder::build(THREAD_FURI->add_component("spawn"))
                                         ->domain_range(THREAD_FURI, {1, 1}, OBJ_FURI, {0, 0})
                                         ->inst_f([](const Obj_p &thread_obj, const InstArgs &) {
                                           ROUTER_READ(*SCHEDULER_ID)->inst_apply("spawn", {thread_obj});
                                           return Obj::to_noobj();
                                         })
                                         ->create()}});
              })
              ->create());
    }
  };

} // namespace fhatos
#endif
