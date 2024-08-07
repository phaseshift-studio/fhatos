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
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <util/enums.hpp>

namespace fhatos {
  enum class SType { READ, WRITE, READWRITE };
  static const Enums<SType> StructureTypes =
      Enums<SType>({{SType::READ, "read"}, {SType::WRITE, "write"}, {SType::READWRITE, "readwrite"}});

  class Structure : public Patterned {
  protected:
    std::atomic_bool _available = std::atomic_bool(false);

  public:
    const SType _stype;

    Structure(const Pattern_p &type, const SType stype) : Patterned(type), _stype(stype) {}

    virtual ~Structure() = default;

    virtual void open() {
      if (this->_available.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
        this->_available.store(true);
      }
    }
    virtual void maintain() {}
    virtual void close() {
      if (this->_available.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already closed");
        this->_available.store(false);
      }
    }
    virtual void write(const Message_p &message) = 0;
    virtual void read(const Subscription_p &subscription) = 0;
    bool available() { return this->_available; }

    static fError ID_NOT_IN_RANGE(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! is not within the boundaries of space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }

    static fError ID_DOES_NOT_EXIST(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! does not reference an obj in space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }
  };

} // namespace fhatos

#endif
