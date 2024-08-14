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
#ifndef fhatos_uuid_hpp
#define fhatos_uuid_hpp

#include <fhatos.hpp>
#include <random>
#include <sstream>

namespace fhatos {
  using uuid = string;
  // TODO: move to string_helper
  class UUID final {
  protected:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::uniform_int_distribution<> dis2;

    UUID() :
        rd(std::random_device()), gen(std::mt19937(rd())), dis(std::uniform_int_distribution<>(0, 15)),
        dis2(std::uniform_int_distribution<>(8, 11)) {}

  public:
    static UUID *singleton() {
      static UUID uuid = UUID();
      return &uuid;
    }

    ptr<uuid> mint(const uint8_t size = 36) {
      std::stringstream ss;
      int i;
      ss << std::hex;
      for (i = 0; i < 8; i++) {
        ss << dis(gen);
      }
      ss << "-";
      for (i = 0; i < 4; i++) {
        ss << dis(gen);
      }
      ss << "-4";
      for (i = 0; i < 3; i++) {
        ss << dis(gen);
      }
      ss << "-";
      ss << dis2(gen);
      for (i = 0; i < 3; i++) {
        ss << dis(gen);
      }
      ss << "-";
      for (i = 0; i < 12; i++) {
        ss << dis(gen);
      };
      return share<uuid>(ss.str().substr(0, size));
    }
  };
} // namespace fhatos

#endif
