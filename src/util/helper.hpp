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
#ifndef fhatos_helper_hpp
#define fhatos_helper_hpp

#include "../fhatos.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  using std::make_pair;
  using std::vector;

  class Helper {
  public:
    Helper() = delete;

    template<typename T>
    static Lst_p transform_vector(const std::vector<T> list, const Function<T, Obj_p> mapping) {
      const auto new_list = std::make_shared<std::vector<Obj_p>>();
      for(const T &t: list) {
        new_list->emplace_back(mapping(t));
      }
      return Obj::to_lst(new_list);
    }

    static Lst_p transform_vector(const std::vector<std::string> list) {
      return Helper::transform_vector<std::string>(list, [](const std::string &s) { return Obj::to_str(s); });
    }
  };
} // namespace fhatos
#endif
