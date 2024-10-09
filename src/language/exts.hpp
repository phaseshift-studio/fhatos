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
#ifndef fhatos_exts_hpp
#define fhatos_exts_hpp


#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>

#include "types.hpp"

namespace fhatos {
  class Exts {
  public:
    Exts() = delete;

    static List<Pair<ID, Type_p>> exts(const ID &extId) {
      static Map_p<ID, List<Pair<ID, Type_p>>> exts =
          std::make_shared<Map<ID, List<Pair<ID, Type_p>>>>(Map<ID, List<Pair<ID, Type_p>>>{
            {"/model/sys", {
                /// PROCESSES
                {"/type/rec/thread", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                {"/type/rec/fiber", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                {"/type/rec/coroutine", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                /// STRUCTURES
                {"/type/rec/local", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                {"/type/rec/network", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                {"/type/rec/external", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
                ///////
                {"/type/inst/stop", OBJ_PARSER("map(noobj).to(*_0)")},
                {"/type/rec/sub", OBJ_PARSER("[:source=>uri[_],:pattern=>uri[_],:qos=>int[_],:on_recv=>_]")},
                {"/type/rec/msg", OBJ_PARSER("[:target=>uri[_],:payload=>_,:retain=>bool[_]]")},
              }
            }});
      return exts->at(extId);
    }

    static void load_extension(const ID &ext_id) {
      for (const auto &[key, value]: Exts::exts(ext_id)) {
        const auto type_id = make_shared<ID>(key);
        const auto value_clone = value->clone();
        Types::singleton()->save_type(type_id, value_clone);
      }
    }
  };
} // namespace fhatos

#endif
