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
#include "../../../../../lang/type.hpp"
#include "../../../../../lang/obj.hpp"
#include "../../../../model.hpp"
#include "../../../../../lang/mmadt/mmadt_obj.hpp"

namespace fhatos {
  using namespace mmadt;
  class Thread;
  static auto this_thread = atomic<Thread *>(nullptr);
  static ID_p THREAD_FURI = id_p(FOS_URI "/thread");

  class Thread : public Model<Thread> {
  public:
    Obj_p thread_obj_;
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

    void delay(uint64_t milliseconds);

    static void delay_current_thread(uint64_t milliseconds) {
      if(current_thread().has_value()) {
        FEED_WATCHDOG();
        current_thread().value()->delay(milliseconds);
      }
    }

    void yield();

    static void yield_current_thread() {
      if(current_thread().has_value()) {
        FEED_WATCHDOG();
        current_thread().value()->yield();
      }
    }

    void halt();

    explicit Thread(const Obj_p &thread_obj, const Consumer<Obj_p> &thread_function = [](const Obj_p &thread_obj) {
                      try {
                        thread_obj->rec_set("halt", dool(false));
                        thread_obj->save("halt");
                        LOG_WRITE(INFO, thread_obj.get(), L("!ythread!! spawned: {} !m[!ystack size:!!{}!m]!!\n",
                                                            thread_obj->rec_get("loop")->toString(),
                                                            thread_obj->rec_get("config/stack_size",
                                                              ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size"))
                                                            )->toString())                            );

                        bool force_halt = false;
                        while(!force_halt && !ROUTER_READ(thread_obj->vid->extend("halt"))->bool_value()) {
                          FEED_WATCHDOG();
                          try {
                            const BCode_p &code = thread_obj->rec_get("loop");
                            force_halt = ROUTER_READ(thread_obj->vid->extend("halt"))->bool_value();
                            mmADT::delift(code)->apply(thread_obj);
                            if(const int delay = thread_obj->get<int>("delay"); delay > 0) {
                              Thread::get_state(thread_obj)->delay(delay);
                              thread_obj->rec_set("delay", jnt(0, NAT_FURI));
                              thread_obj->save("delay");
                            }
                          } catch(const std::exception &e) {
                            LOG_WRITE(ERROR, thread_obj.get(),L("!rthread error!!: {}", e.what()));
                            force_halt = true;
                          }
                         // thread_obj->sync();
                        }
                        try {
                          Thread::get_state(thread_obj)->halt();
                          ROUTER_WRITE(thread_obj->vid->extend("halt"), dool(true), true);
                          MODEL_STATES::singleton()->remove(*thread_obj->vid);
                        } catch(const std::exception &e) {
                          MODEL_STATES::singleton()->remove(*thread_obj->vid);
                          throw fError::create(thread_obj->vid->toString(), "unable to stop thread: %s",
                                               e.what());
                        }
                        LOG_WRITE(INFO, thread_obj.get(), L("!ythread!! stopped\n"));
                      } catch(std::exception &e) {
                        MODEL_STATES::singleton()->remove(*thread_obj->vid);
                        throw fError::create(thread_obj->vid->toString(), "unable to process thread: %s", e.what());
                      }
                    });

    static ptr<Thread> create_state(const Obj_p &thread_obj) {
      return make_shared<Thread>(thread_obj);
    }

    static void *import() {
      const Rec_p thread_t = Obj::to_rec({
          {"loop", __()},
          {"delay", __().else_(jnt(0,NAT_FURI))},
          {"halt", __().else_(dool(true))}
      });
      Typer::singleton()->save_type(*THREAD_FURI, thread_t);
      InstBuilder::build(THREAD_FURI->add_component("spawn"))
          ->domain_range(THREAD_FURI, {1, 1}, OBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &thread_obj, const InstArgs &) {
            ROUTER_READ(*SCHEDULER_ID)->inst_apply("spawn", {thread_obj});
            return Obj::to_noobj();
          })->save();
      return nullptr;
    }
  };

} // namespace fhatos
#endif
