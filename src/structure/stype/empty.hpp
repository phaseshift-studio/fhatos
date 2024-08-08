//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#ifndef fhatos_empty_hpp
#define fhatos_empty_hpp

#include <fhatos.hpp>
#include <structure/structure.hpp>


namespace fhatos {
  class Empty : public Structure {

  public:
    explicit Empty(const Pattern &pattern = EMPTY_CHARS) : Structure(pattern, SType::READ) {}
    virtual void setup() override {};
    virtual void stop() override {};
    virtual void loop() override {};
    virtual List<Pair<fURI_p, Obj_p>> read(const fURI &furi, const ID &source) override { return {}; }
    virtual void write(const fURI &furi, const Obj_p &obj, const ID &source) override {}
  };
} // namespace fhatos

#endif
