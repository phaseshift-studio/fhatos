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
#ifndef fhatos_id_structure_hpp
#define fhatos_id_structure_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/structure.hpp>

namespace fhatos {
  class IDStructure : public Structure {
  protected:
    Obj_p id_obj_ = noobj();

    explicit IDStructure(const Pattern &id) : Structure(id, SType::EPHEMERAL) {
    }

  public:
    static ptr<IDStructure> create(const Pattern_p &id) {
      auto id_structure_p = ptr<IDStructure>(new IDStructure(*id));
      return id_structure_p;
    }

    void write(const ID_p &id, const Obj_p &obj, const bool retain) override {
      Structure::write(id, obj, retain);
      this->loop();
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj) override {
      if (this->pattern()->equals(*id))
        this->id_obj_ = obj;
      //distribute_to_subscribers(share(Message{.target = *id, .payload = obj, .retain = true}));
    }

    List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &furi) override {
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      return furi->matches(*this->pattern())
               ? List<Pair<ID_p, Obj_p>>({{id_p(*this->pattern()), this->id_obj_}})
               : List<Pair<ID_p, Obj_p>>();
    }
  };
} // namespace fhatos
#endif
