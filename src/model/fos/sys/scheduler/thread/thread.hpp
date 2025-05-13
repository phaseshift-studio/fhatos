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
#include "../../../../../lang/type.hpp"
#include "../../../../fos/sys/memory/memory.hpp"
#include "../../../../model.hpp"

namespace fhatos {
  using namespace mmadt;
  class Thread;
  static auto this_thread = atomic<Thread *>(nullptr);
  static ID_p THREAD_FURI = id_p(FOS_URI "/sys/thread");

  class Thread {
  public:
    Consumer<std::pair<Thread *, Obj_p>> thread_function_;
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
        const Obj_p &thread_obj,
        const Consumer<std::pair<Thread *, Obj_p>> &thread_function =
            [](const std::pair<const Thread *, const Obj_p &> &pair) {
              auto [thread_ptr, thread_obj] = pair;
              try {
                // const ptr<Thread> current = Thread::get_state(thread_obj);
                thread_obj->obj_set("halt", dool(false));
                const Obj_p thread_loop_obj =
                    thread_obj->is_rec() && !thread_obj->rec_get("loop")->is_noobj()
                        ? thread_obj->obj_get("loop")
                        : Compiler(false, false)
                              .resolve_inst(thread_obj, Obj::to_inst(Obj::to_inst_args(), id_p("loop"),
                                                                     id_p(thread_obj->vid->extend("loop"))));
                const Inst_p thread_loop_inst = mmADT::delift(thread_loop_obj);
                LOG_WRITE(INFO, thread_obj.get(),
                          L("!g[!bfhatos!g] !ythread!! spawned: {} !m[!ystack size:!!{}!m]!!\n",
                            thread_loop_inst->toString(), thread_obj->obj_get("config/stack_size")->toString()));
                while(!thread_obj->obj_get("halt")->or_else_<bool>(false)) {
                  FEED_WATCHDOG();
                  try {
                    thread_loop_inst->apply(thread_obj);
                    if(const int delay = thread_obj->obj_get("delay")->or_else_<int>(0); delay > 0) {
                      Thread::delay(delay);
                      thread_obj->obj_set("delay", jnt(0, NAT_FURI));
                    }
                  } catch(const fError &e) {
                    LOG_WRITE(ERROR, thread_obj.get(), L("!rthread loop error!!: {}\n", e.what()));
                  }
                }
                return Obj::to_noobj();
              } catch(const fError &e) {
                LOG_WRITE(ERROR, thread_obj.get(), L("!rthread construction error!!: {}\n", e.what()));
              }
              thread_obj->delete_model();
              thread_obj->obj_set("halt", dool(true));
              LOG_WRITE(INFO, thread_obj.get(), L("!ythread!! stopped\n"));
              thread_ptr->halt();
            });

    static ptr<Thread> create_state(const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); }


    static void *import() {
      MODEL_CREATOR2->insert_or_assign(*THREAD_FURI,
                                       [](const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); });
      const Rec_p thread_t = Obj::to_rec({{"loop", __()}, {"halt", __().else_(dool(true))}});
      Typer::singleton()->save_type(*THREAD_FURI, thread_t);
      InstBuilder::build(THREAD_FURI->add_component("spawn"))
          ->domain_range(THREAD_FURI, {1, 1}, OBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &thread_obj, const InstArgs &) {
            ROUTER_READ(*SCHEDULER_ID)->inst_apply("spawn", {thread_obj});
            return Obj::to_noobj();
          })
          ->save();
      return nullptr;
    }
  };

} // namespace fhatos
#endif
