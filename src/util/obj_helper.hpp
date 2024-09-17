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
#ifndef fhatos_obj_helper_hpp
#define fhatos_obj_helper_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {
  class ObjHelper final {
  public:
    ObjHelper() = delete;

    static const string objAnalysis(const Obj &obj) {
      char a[250];
      sprintf(a,
              "!b%s!! structure:\n"
              "\t!gid!!            : %s\n"
              "\t!grange<=domain!! : %s<=%s\n"
              "\t!gsize!!  (bytes) : %i\n"
              "\t!gbcode!!         : %s\n"
              "\t!gvalue!!         : %s",
              obj.id()->name().c_str(), obj.id()->toString().c_str(), OTypes.to_chars(obj.o_type()).c_str(),
              OTypes.to_chars(obj.o_type()).c_str(), obj.serialize()->first, FOS_BOOL_STR(obj.is_bcode()),
              obj.is_bcode() ? obj.toString().c_str() : obj.toString(false).c_str());
      return string(a);
    }
  };
} // namespace fhatos
#endif
