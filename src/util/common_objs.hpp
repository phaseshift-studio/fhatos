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
#ifndef fhatos_common_objs_hpp
#define fhatos_common_objs_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>

namespace fhatos {
  class CommonObjs {
  public:
    static Rec_p terminal(const ID &id) {
      return Obj::to_rec({
                             {vri(":stdout"), Obj::to_bcode([](const Obj_p &obj) {
                               printer<>()->print(obj->str_value().c_str());
                               return noobj();
                             }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
                             {vri(":stdin"), Obj::to_bcode([](const NoObj_p &) {
#ifdef NATIVE
                               return jnt(getchar());
#else
          return jnt((Serial.available() > 0) ? Serial.read() : EOF);
#endif
                             }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))}}, REC_FURI, id_p(id));
    }
  };
} // namespace fhatos
#endif