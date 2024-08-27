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
#ifndef fhatos_empty_hpp
#define fhatos_empty_hpp

#include <fhatos.hpp>
#include <structure/structure.hpp>


namespace fhatos {
  class Empty : public Structure {

  protected:
    explicit Empty(const Pattern & = Pattern(EMPTY_CHARS)) : Structure("", SType::READ) {}

  public:
    static ptr<Empty> singleton() {
      static ptr<Empty> empty_p = ptr<Empty>(new Empty());
      return empty_p;
    }

    virtual Obj_p read(const fURI_p &, const ID_p &) override { return Objs::to_objs(); }

    virtual void write(const ID_p &id, const Obj_p &obj, [[maybe_unused]] const ID_p &source) override {
      LOG_STRUCTURE(INFO, this, "write attempted: %s -- %s\n", id->toString().c_str(), obj->toString().c_str());
    }
  };
} // namespace fhatos

#endif
