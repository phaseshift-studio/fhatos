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

  class Thread : public Obj {
  public:
    Consumer<Obj_p> thread_function_;
    Any handler_;

    static Option<Thread *> current_thread() {
      if(this_thread.load())
        return {this_thread.load()};
      else {
        return {};
      }
    }

    template<typename HANDLER>
    HANDLER get_handler() {
      return std::any_cast<HANDLER>(this->handler_);
    }

    static int core_current_thread() {
#ifdef ESP_PLATFORM
      return xPortGetCoreID();
#else
      return sched_getcpu();
#endif
    }

    void delay(uint64_t milliseconds);

    static void delay_current_thread(uint64_t milliseconds) {
#ifdef ESP_PLATFORM
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
#else
      std::this_thread::sleep_for(chrono::milliseconds(milliseconds));
#endif
    }

    void yield();

    static void yield_current_thread() {
#ifdef ESP_PLATFORM
      // vTaskDelay(1 / portTICK_PERIOD_MS);
      taskYIELD();
#else
      std::this_thread::yield();
#endif
    }

    void halt();

    ptr<Thread> shared_from_this() {
      return std::dynamic_pointer_cast<Thread>(const_pointer_cast<Obj>(Obj::shared_from_this()));
    }

    explicit Thread(
        const Obj_p &thread_obj, const Consumer<Obj_p> &thread_function = [](const Obj_p &thread_obj) {
          try {
            // const ptr<Thread> current = Thread::get_state(thread_obj);
            thread_obj->obj_set("halt", dool(false));
            const int stack_size =
                Memory::get_stack_size(thread_obj, "config/stack_size",
                                       ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size"))->or_else_(0));
            thread_obj->obj_set("config/stack_size", jnt(stack_size));
            const Obj_p thread_loop_obj =
                thread_obj->is_rec() && thread_obj->has("loop")
                    ? thread_obj->obj_get("loop")
                    : Compiler(false, false)
                          .resolve_inst(thread_obj, Obj::to_inst(Obj::to_inst_args(), id_p("loop"),
                                                                 id_p(thread_obj->vid->extend("loop"))));
            const Inst_p thread_loop_inst = mmADT::delift(thread_loop_obj);
            LOG_WRITE(INFO, thread_obj.get(),
                      L("!g[!bfhatos!g] !ythread!! spawned: {} !m[!ystack size:!!{}!m]!!\n",
                        thread_loop_inst->toString(), thread_obj->obj_get("config/stack_size")->toString()));
            while(!thread_obj->obj_get("halt")->or_else_(false)) {
              FEED_WATCHDOG();
              try {
                thread_loop_inst->apply(thread_obj);
              } catch(const fError &e) {
                LOG_WRITE(ERROR, thread_obj.get(), L("!rthread error!!: {}\n", e.what()));
              }
            }
            try {
              thread_obj->get_model<Thread>()->halt();
              thread_obj->delete_model();

            } catch(const fError &e) {
              thread_obj->delete_model();
              throw fError::create(thread_obj->vid->toString(), "unable to stop thread: %s", e.what());
            }
            LOG_WRITE(INFO, thread_obj.get(), L("!ythread!! stopped\n"));
          } catch(const fError &e) {
            thread_obj->delete_model();
            throw fError::create(thread_obj->vid->toString(), "unable to process thread: %s", e.what());
          }
        });

    static ptr<Thread> create_state(const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); }


    static void *import() {
      MODEL_CREATOR2->insert_or_assign(*THREAD_FURI, [](const Obj_p &thread_obj) { return make_shared<Thread>(thread_obj); });
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
