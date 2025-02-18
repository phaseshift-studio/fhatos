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
#ifndef fhatos_fthread_hpp
#define fhatos_fthread_hpp
#include "../../../../fhatos.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../model.hpp"
#include "fmutex.hpp"

namespace fhatos {
  static ID_p THREADX_FURI = id_p(FOS_URI "/sys/threadx");
  static ID_p THREADX_FURI_DEFAULT = id_p(FOS_URI "/sys/threadx::default");

  class fThread : public Model<fThread> {
  public:
    Obj_p thread_obj_;
    Consumer<Obj_p> thread_function_;
    Any handler_;

    template<typename HANDLER>
    HANDLER get_handler() {
      return std::any_cast<HANDLER>(this->handler_);
    }

    void delay(uint64_t milliseconds);

    void yield();

    void halt();

    explicit fThread(const Obj_p &thread_obj, const Consumer<Obj_p> &thread_function =
                           [](const Obj_p &thread_obj)  {
                         try {
                           const ptr<fThread> thread_state = fThread::get_state(thread_obj);
                           const Obj_p loop_code = thread_obj->rec_get("loop");
                           LOG_OBJ(INFO, thread_obj, "!ythread!! spawned: %s\n", loop_code->toString().c_str());
                           while(!thread_obj->get<bool>("halt")) {
                             const BCode_p &code = thread_obj->rec_get("loop");
                             code->apply(thread_obj);
                             if(const int delay = thread_obj->get<int>("delay"); delay > 0) {
                               thread_state->delay(delay);
                               thread_obj->rec_set("delay", jnt(0, NAT_FURI));
                             }
                           }
                           try {
                             thread_state->halt();
                             MODEL_STATES::singleton()->remove(*thread_obj->vid);
                           } catch(const std::exception &e) {
                             MODEL_STATES::singleton()->remove(*thread_obj->vid);
                             throw fError::create(thread_obj->vid->toString(), "unable to stop thread: %s", e.what());
                           }
                           LOG_OBJ(INFO, thread_obj, "!ythread!! stopped\n");
                         } catch(std::exception &e) {
                           MODEL_STATES::singleton()->remove(*thread_obj->vid);
                           throw fError::create(thread_obj->vid->toString(), "unable to process thread: %s", e.what());
                         }
                       });

    static ptr<fThread> create_state(const Obj_p &thread_obj) {
      return make_shared<fThread>(thread_obj);
    }

    static Obj_p start_inst(const Obj_p &thread_obj, const InstArgs &args) {
      fThread::get_state(thread_obj);
      return thread_obj;
    }

    static void *import() {
      Typer::singleton()->save_type(*THREADX_FURI, Obj::to_rec({
                                      {"loop", Obj::to_bcode()},
                                      {"delay", Obj::to_type(NAT_FURI)},
                                      {"halt", Obj::to_type(BOOL_FURI)}
                                    }));
      Typer::singleton()->save_type(*THREADX_FURI_DEFAULT, Obj::to_rec({
                                      {"loop", Obj::to_bcode()},
                                      {"delay", jnt(0, NAT_FURI)},
                                      {"halt", dool(false)}
                                    }));

      InstBuilder::build(THREADX_FURI->add_component("spawn"))
          ->domain_range(THREADX_FURI, {1, 1}, THREADX_FURI, {1, 1})
          ->inst_f([](const Obj_p &thread_obj, const InstArgs &args) {
            return fThread::start_inst(thread_obj, args);
          })->save();
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
