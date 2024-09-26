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
#ifndef fhatos_obj_structure_hpp
#define fhatos_obj_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/router.hpp>
#include <structure/stype/key_value.hpp>

#include "router.hpp"

namespace fhatos {
  class KeyValueObj : public KeyValue {
    const Rec_p kv_rec_;

  public:
    explicit KeyValueObj(const Pattern &pattern): KeyValue(pattern) {
    }

    void setup() override {
      try {
        KeyValue::setup();
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }
  };

  inline void load_attacher() {
    STRUCTURE_ATTACHER = [](const Pattern &structure_pattern) {
      Router::singleton()->attach(std::make_shared<KeyValueObj>(structure_pattern));
    };
  }
} // namespace fhatos
#endif
