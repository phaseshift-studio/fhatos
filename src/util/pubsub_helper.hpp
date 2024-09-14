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
#ifndef pubsub_helper_hpp
#define pubsub_helper_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/structure.hpp>

namespace fhatos {
  class PubSubHelper final {
  public:
    PubSubHelper() = delete;

    static Function<const ID_p, Obj_p> read_selector(const Map<Pattern, Function<ID_p, Obj_p>> &map) {
      return [map](const ID_p &target) {
        for (const auto &[pattern,con]: map) {
          if (target->matches(pattern))
            return con(target);
        }
        return noobj();
      };
    }
  };
} // namespace fhatos

#endif
