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
#ifndef fhatos_enumerator_hpp
#define fhatos_enumerator_hpp

#include <list>
#include <string>

using namespace std;
namespace fhatos {
  template<typename ENUM>
  struct Enums {
    const list<pair<ENUM, const char *>> ENUM_TO_STR;
    Enums(const initializer_list<pair<ENUM, const char *>> &enums) : ENUM_TO_STR(enums) {}

    const char *toChars(const ENUM e) const {
      for (const auto &pair: ENUM_TO_STR) {
        if (pair.first == e)
          return pair.second;
      }
      throw fError("toChars enum not found: %i\n", e);
    }

    ENUM toEnum(const char *s) const {
      for (const auto &pair: ENUM_TO_STR) {
        if (strcmp(pair.second, s) == 0)
          return pair.first;
      }
      throw fError("toEnum enum not found: %s\n", s);
    }
  };
} // namespace fhatos

#endif
