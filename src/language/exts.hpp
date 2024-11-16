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
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/type.hpp>

namespace fhatos {
  // static ID inst_id(const string &opcode) { return INST_FURI->resolve(opcode); }
  const Str_p ARG_ERROR = str("wrong number of arguments");
  static auto MODELS = std::make_shared<Map<ID, List<Pair<ID, Obj_p>>>>(Map<ID, List<Pair<ID, Obj_p>>>{
      {"/model/sys/",
       {{"/type/rec/heap",
         Obj::to_rec(
             {{vri(":setup"), Obj::to_bcode()}, {vri(":loop"), Obj::to_bcode()}, {vri(":stop"), Obj::to_bcode()}})},
        {"/type/rec/computed",
         Obj::to_rec(
             {{vri(":setup"), Obj::to_bcode()}, {vri(":loop"), Obj::to_bcode()}, {vri(":stop"), Obj::to_bcode()}})},
        {"/type/rec/mqtt",
         Obj::to_rec(
             {{vri(":setup"), Obj::to_bcode()}, {vri(":loop"), Obj::to_bcode()}, {vri(":stop"), Obj::to_bcode()}})}}}});

  class Exts {
  public:
    Exts() = delete;

    static void load_extension(const ID &ext_id) {
      const List<Pair<ID, Obj_p>> &pairs = MODELS->at(ext_id);
      Type::singleton()->start_progress_bar(pairs.size());
      for (const auto &[key, value]: pairs) {
        const auto type_id = id_p(key);
        const auto value_clone = value->clone();
        Type::singleton()->save_type(type_id, value_clone);
      }
      Type::singleton()->end_progress_bar(StringHelper::format("!b%s !yobjs!! loaded\n", ext_id.toString().c_str()));
    }
  };
} // namespace fhatos

#endif