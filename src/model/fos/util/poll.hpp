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
#ifndef fhatos_util_poll_hpp
#define fhatos_util_poll_hpp

#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../model.hpp"
#include "../../../lang/type.hpp"
#include "../sys/scheduler/thread/thread.hpp"

namespace fhatos {
  const ID_p POLL_FURI = id_p(FOS_URI "/util/poll");

  class Poll final : public Thread {

  public:
    explicit Poll(const Obj_p &poll_obj, const Consumer<Obj_p> &function) :
      Thread(poll_obj, function) {
    }

    static ptr<Poll> get_state(const Obj_p &poll_obj) {
      return Model::get_state<Poll>(poll_obj);
    }

    static ptr<Poll> create_state(const Obj_p &poll_obj) {
      return make_shared<Poll>(poll_obj, [](const Obj_p &poll_obj) {
        try {
          const auto start_time = std::chrono::high_resolution_clock::now();
          LOG_WRITE(INFO, poll_obj.get(), L("!ypolling !b{} !ystarted!! [delay:{} ms]\n",
                  poll_obj->rec_get("loop")->toString(),
                  poll_obj->get<int>("delay")));
          while(!poll_obj->get<bool>("halt")) {
            const Obj_p code = poll_obj->rec_get("loop");
            const Obj_p result = BCODE_PROCESSOR(code);
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_obj->get<int>("delay")));
          }
          const std::chrono::duration<double, milli> duration = std::chrono::high_resolution_clock::now() - start_time;
          LOG_WRITE(INFO, poll_obj.get(), L("!ypolling !b{} !ystopped!! [runtime:{} sec]\n",
                  poll_obj->rec_get("loop")->toString(),
                  duration.count() / 1000.0f));
        } catch(const std::exception &e) {
          LOG_EXCEPTION(poll_obj, fError("poll failure: %s", e.what()));
        }
      });
    }

    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      Typer::singleton()->save_type(
          *POLL_FURI, Obj::to_rec({
              {"delay", Obj::to_type(MILLISECOND_FURI)},
              {"loop", Obj::to_bcode()},
              {"halt", Obj::to_type(BOOL_FURI)}
          }));
      InstBuilder::build(POLL_FURI->add_component("poll"))
          ->inst_f([](const Obj_p &poll, const InstArgs &) {
            Poll::get_state(poll);
            return poll;
          })
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
