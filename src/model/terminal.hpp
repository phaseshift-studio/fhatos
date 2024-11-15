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
#ifndef fhatos_terminal_hpp
#define fhatos_terminal_hpp

#include <fhatos.hpp>
#include <structure/stype/heap.hpp>

namespace fhatos {
  class Terminal final : public Rec {


  protected:
    explicit Terminal(const ID &id) :
      Rec(rmap({{":stdout", Obj::to_bcode(
                     [](const Str_p &obj) {
                       FEED_WATCDOG();
                       STD_OUT_DIRECT(obj);
                       return noobj();
                     },
                     StringHelper::cxx_f_metadata(__FILE__, __LINE__))},
                {":stdin", Obj::to_bcode(
                     [](const NoObj_p &) {
                       return STD_IN_DIRECT();
                     },
                     StringHelper::cxx_f_metadata(__FILE__, __LINE__))}}),
          OType::REC, id_p(REC_FURI->extend("terminal")), id_p(id)) {
    }

  public:
    static ptr<Terminal> singleton(const ID &id = ID("/io/terminal")) {
      static bool setup = false;
      if (!setup) {
        setup = true;
        Type::singleton()->save_type(id_p(REC_FURI->extend("terminal")),
                                     rec({{vri(":stdout"), Obj::to_bcode()}, {vri(":stdin"), Obj::to_bcode()}}));
      }
      static auto terminal_p = ptr<Terminal>(new Terminal(id));
      return terminal_p;
    }

    static void STD_OUT_DIRECT(const Str_p &str) {
      static auto STDOUT_MUTEX = std::mutex();
      std::lock_guard<std::mutex> lock(STDOUT_MUTEX);
      printer<>()->print(str->str_value().c_str());
    }

    static Int_p STD_IN_DIRECT() {
      int c = -1;
      while (-1 == (c = printer<>()->read())) {
        Process::current_process()->yield();
      }
      return jnt(c);
    }
  };
} // namespace fhatos
#endif