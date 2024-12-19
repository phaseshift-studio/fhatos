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

#include "string_helper.hpp"

namespace fhatos {
  using std::list;
  using std::initializer_list;
  using std::pair;

  template<typename ENUM>
  struct Enums {
    const list<pair<ENUM, string>> ENUM_TO_STR{};

    explicit Enums(const initializer_list<pair<ENUM, string>> &enums) : ENUM_TO_STR(enums) {
    }

    bool has_enum(const string &s) const {
      for(const auto &pair: ENUM_TO_STR) {
        if(s == pair.second)
          return true;
      }
      return false;
    }

    string to_chars(const ENUM e) const {
      for(const auto &pair: ENUM_TO_STR) {
        if(pair.first == e)
          return pair.second;
      }
      throw fError("!ychars!! not found for enum !b%i!!", e);
    }

    ENUM to_enum(const string &s) const {
      for(const auto &pair: ENUM_TO_STR) {
        if(s == pair.second)
          return pair.first;
      }
      string enums_string;
      for(const auto &pair: ENUM_TO_STR) {
        enums_string += (pair.second + " ");
      }
      StringHelper::trim(enums_string);
      //LOG(ERROR,"!yenum!! not found for chars !y%s!! [!b%s!!]", s.c_str(), enums_string.c_str());
      return ENUM_TO_STR.front().first;
      //throw fError("!yenum!! not found for chars !y%s!! [!b%s!!]", s.c_str(), enums_string.c_str());
    }
  };
} // namespace fhatos

#endif
