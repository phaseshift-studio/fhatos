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

#include <future>
#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../interface.hpp"
#include "../../model.hpp"
#include "../../../lang/type.hpp"

namespace fhatos {
  const ID_p POLL_FURI = id_p("/fos/util/poll");

  class Poll final : public Model<Poll> {
    std::thread poll_thread;

  public:
    static ptr<Poll> create_state(const Obj_p &) {
      return make_shared<Poll>();
    }

    static Obj_p poll_inst(const Obj_p &poll, const InstArgs &args) {
      const ptr<Poll> poll_state = Poll::get_or_create(poll);
      poll_state->poll_thread = std::thread([](const ID &poll_id) {
        Obj_p poll = Obj::load(poll_id);
        try {
          const auto start_time = std::chrono::high_resolution_clock::now();
          LOG_OBJ(INFO, poll, "!ypolling !b%s !ystarted!! [freq:%i ms]\n",
                  poll->rec_get("code")->toString().c_str(),
                  poll->rec_get("freq")->int_value());
          while(!poll->rec_get("halt")->bool_value()) {
            const Obj_p code = poll->rec_get("code");
            const Obj_p result = BCODE_PROCESSOR(code);
            std::this_thread::sleep_for(std::chrono::milliseconds(poll->rec_get("freq")->int_value()));
            poll = Obj::load(poll_id);
          }
          const std::chrono::duration<double,milli> duration = std::chrono::high_resolution_clock::now() - start_time;
          LOG_OBJ(INFO, poll, "!ypolling !b%s !ystopped!! [runtime:%f sec]\n",
                  poll->rec_get("code")->toString().c_str(),
                  duration.count() * 1000.0f);
        } catch(const std::exception &e) {
          LOG_EXCEPTION(poll, fError::create(poll_id.toString(), "poll failure: %s", e.what()));
        }
      }, *poll->vid);

      GLOBAL::singleton()->store(poll->vid, poll_state);
      return poll;
    }


    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      Typer::singleton()->save_type(
        POLL_FURI, Obj::to_rec({
          {"freq", Obj::to_type(MILLISECOND_FURI)},
          {"code", Obj::to_bcode()},
          {"halt", Obj::to_type(BOOL_FURI)}
        }));
      InstBuilder::build(POLL_FURI->add_component("poll"))
          ->inst_f([](const Obj_p &poll, const InstArgs &args) {
            return Poll::poll_inst(poll, args);
          })
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
