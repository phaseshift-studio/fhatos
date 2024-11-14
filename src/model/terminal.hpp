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
  private:
    std::mutex stdout_mutex = std::mutex();

  protected:
    explicit Terminal(const ID &id) :
      Rec(rmap({{":stdout", Obj::to_bcode(
                     [this](const Str_p &obj) {
                       FEED_WATCDOG();
                       std::lock_guard<std::mutex> lock(stdout_mutex);
                       printer<>()->print(obj->str_value().c_str());
                       return noobj();
                     },
                     StringHelper::cxx_f_metadata(__FILE__, __LINE__))},
                {":stdin", Obj::to_bcode(
                     [](const NoObj_p &) {
#ifdef NATIVE
                       return jnt(getchar());
#else
                                   while (Serial.available() <= 0) {
                                     Process::current_process()->yield();
                                   }
                                   return jnt(Serial.read());
      // return jnt((Serial.available() > 0) ? Serial.read() : EOF); (need a MACRO for multi-core checking)
#endif
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
  };
} // namespace fhatos
#endif