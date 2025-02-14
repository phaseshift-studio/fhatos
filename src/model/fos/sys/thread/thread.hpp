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
#ifndef fhatos_sys_thread_hpp
#define fhatos_sys_thread_hpp
#include "../../../../fhatos.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../model.hpp"
#include STR(HARDWARE/threadxx.hpp)

namespace fhatos {
  static ID_p THREADX_FURI = id_p("/fos/threadx");
  static ID_p THREADX_FURI_DEFAULT = id_p("/fos/threadx::default");

  class ThreadX : public Model<ThreadX> {
  public:
    ThreadXX threadxx;

    explicit ThreadX(const Obj_p &thread_obj, const Consumer<Obj_p> &function) : threadxx(function,thread_obj) {
    }

    static ptr<ThreadX> create_state(const Obj_p &thread_obj) {
      return make_shared<ThreadX>(thread_obj, [](const Obj_p &thread_obj) -> void {
        try {
        const auto thread_state = ThreadX::get_state(thread_obj);
        const Obj_p loop_code = thread_obj->rec_get("loop");
        LOG_OBJ(INFO, thread_obj, "!ythread!! spawned: %s\n", loop_code->toString().c_str());
        //Obj_p running = thread_obj;
        while(true) {
          const Obj_p thread_obj_fresh = thread_obj->load();
          thread_obj_fresh->rec_get("loop")->apply(thread_obj_fresh);
          if(const int delay = thread_obj_fresh->rec_get("delay")->or_else(jnt(0))->int_value(); delay > 0) {
            thread_state->threadxx.delay(delay);
          }
          if(thread_obj_fresh->rec_get("halt")->bool_value()) {
            try {
              thread_state->threadxx.stop();
              MODEL_STATES::singleton()->remove(*thread_obj->vid);
              break;
            } catch(const std::runtime_error &e) {
              MODEL_STATES::singleton()->remove(*thread_obj->vid);
              throw fError::create(thread_obj->vid->toString(), "unable to halt thread: %s", e.what());
            }
          }
        }
        LOG_OBJ(INFO, thread_obj, "!ythread!! stopped\n");
        } catch(std::exception &e) {
          MODEL_STATES::singleton()->remove(*thread_obj->vid);
            throw fError::create(thread_obj->vid->toString(), "unable to process thread: %s", e.what());
        }
      });
    }

    static Obj_p start_inst(const Obj_p &thread_obj, const InstArgs &args) {
      const ptr<ThreadX> threadx = ThreadX::get_state(thread_obj);
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
            return ThreadX::start_inst(thread_obj, args);
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
