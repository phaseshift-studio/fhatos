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
#include "../../../../../util/obj_helper.hpp"
#include "../../../../model.hpp"
#include "../../../../../lang/mmadt/mmadt_obj.hpp"

namespace fhatos {
  using namespace mmadt;
  class Thread;
  static auto this_thread = atomic<const Thread *>(nullptr);
  static ID_p THREAD_FURI = id_p(FOS_URI "/thread");
  static ID_p THREAD_FURI_DEFAULT = id_p(FOS_URI "/thread::default");

  class Thread : public Model<Thread> {
  public:
    Obj_p thread_obj_;
    Consumer<Obj_p> thread_function_;
    Any handler_;

    static Option<const Thread *> current_thread() {
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

    void yield();

    void halt();

    explicit Thread(const Obj_p &thread_obj, const Consumer<Obj_p> &thread_function = [](const Obj_p &thread_obj) {
                       try {
                         const Obj_p loop_code = thread_obj->rec_get("loop");
                         LOG_WRITE(INFO, thread_obj.get(), L("!ythread!! spawned: {}\n", loop_code->toString()));
                         const ptr<Thread> thread_state = get_state<Thread>(thread_obj);
                         while(!thread_obj->get<bool>("halt")) {
                           FEED_WATCHDOG();
                           this_thread.store(Thread::get_state(thread_obj).get());
                           const BCode_p &code = thread_obj->rec_get("loop");
                           code->apply(thread_obj);
                           if(const int delay = thread_obj->get<int>("delay"); delay > 0) {
                             Thread::get_state(thread_obj)->delay(delay);
                             thread_obj->rec_set("delay", jnt(0, NAT_FURI));
                           }
                         }
                         Lst_p threads = ROUTER_READ(SCHEDULER_ID->extend("thread"));
                         threads->lst_remove(vri(thread_obj->vid));
                         ROUTER_WRITE(SCHEDULER_ID->extend("thread"), threads, true);
                         try {
                           thread_state->halt();
                           MODEL_STATES::singleton()->remove(*thread_obj->vid);
                         } catch(const std::exception &e) {
                           MODEL_STATES::singleton()->remove(*thread_obj->vid);
                           throw fError::create(thread_obj->vid->toString(), "unable to stop thread: %s", e.what());
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
      Typer::singleton()->save_type(*THREAD_FURI, Obj::to_rec({
                                        {"loop", Obj::to_bcode()},
                                        {"delay", Obj::to_type(NAT_FURI)},
                                        {"halt", Obj::to_type(BOOL_FURI)}
                                    }));
      Typer::singleton()->save_type(*THREAD_FURI_DEFAULT, Obj::to_rec({
                                        {"loop", Obj::to_bcode()},
                                        {"delay", jnt(0, NAT_FURI)},
                                        {"halt", dool(false)}
                                    }));

      /*InstBuilder::build(THREAD_FURI->add_component("spawn"))
          ->domain_range(THREAD_FURI, {1, 1}, THREAD_FURI, {1, 1})
          ->inst_f([](const Obj_p &thread_obj, const InstArgs &args) {
            return Thread::start_inst(thread_obj, args);
          })->save();*/
      /* InstBuilder::build(THREADX_FURI->add_component("stop"))
           ->domain_range(THREADX_FURI, {1, 1}, NOOBJ_FURI, {0, 0})
           ->inst_f([](const Obj_p &thread_obj, const InstArgs &args) {
             return ThreadX::stop_inst(thread_obj, args);
           })->save();
       InstBuilder::build(THREADX_FURI->add_component("yield"))
           ->domain_range(THREADX_FURI, {1, 1}, THREADX_FURI, {1, 1})
           ->inst_f([](const Obj_p &thread_obj, const InstArgs &args) {
             return ThreadX::yield_inst(thread_obj, args);
           })->save();
       InstBuilder::build(THREADX_FURI->add_component("delay"))
           ->domain_range(THREADX_FURI, {1, 1}, THREADX_FURI, {1, 1})
           ->inst_f([](const Obj_p &thread_obj, const InstArgs &args) {
             return ThreadX::delay_inst(thread_obj, args);
           })->save();*/
      return nullptr;
    }
  };
} // namespace fhatos
#endif
