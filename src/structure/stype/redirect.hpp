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
#ifndef fhatos_redirect_hpp
#define fhatos_redirect_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/structure.hpp"

namespace fhatos {
  class Redirect : public Structure {
  protected:
    Structure_p base_structure_;
    BCode_p uri_mapper_;

    explicit Redirect(const Pattern &pattern, const Structure_p &base_structure,
                      const BCode_p &uri_mapper) : Structure(pattern, base_structure->stype),
                                                   base_structure_(base_structure),
                                                   uri_mapper_(uri_mapper) {
    }

  public:
    static ptr<Redirect> create(const Pattern &pattern, const Structure_p &base_structure, const BCode_p &uri_mapper) {
      auto redirect_p = ptr<Redirect>(new Redirect(pattern, base_structure, uri_mapper));
      return redirect_p;
    }

    void setup() override {
      Structure::setup();
      if (!this->base_structure_->available())
        this->base_structure_->setup();
    }

    void stop() override {
      if (this->base_structure_->available())
        this->base_structure_->stop();
      Structure::stop();
    }

  protected:
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      this->base_structure_->write_raw_pairs(id_p(this->uri_mapper_->apply(vri(id))->uri_value()), obj, retain);
    }

    List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &match) override {
      return this->base_structure_->read_raw_pairs(furi_p(this->uri_mapper_->apply(vri(match))->uri_value()));
    }
  };
} // namespace fhatos

#endif
