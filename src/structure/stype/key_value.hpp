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
#ifndef fhatos_key_value_hpp
#define fhatos_key_value_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/structure.hpp"

namespace fhatos {

  class KeyValue : public Structure {

  protected:
    Map<ID_p, const Obj_p, furi_p_less> *DATA = new Map<ID_p, const Obj_p, furi_p_less>();
    MutexRW<> MUTEX_DATA = MutexRW<>();

    explicit KeyValue(const Pattern &pattern) : Structure(pattern, SType::READWRITE){};

  public:
    static KeyValue *create(const Pattern &pattern) { return new KeyValue(pattern); }

    ~KeyValue() override { delete DATA; }

    void stop() override {
      Structure::stop();
      DATA->clear();
    }

    void write(const ID_p &target, const Obj_p &payload, const ID &source) override {
      if (DATA->count(target)) {
        DATA->erase(target);
      }
      DATA->insert({target, payload});
    }

    Obj_p read(const ID_p &id, [[maybe_unused]] const ID &source) override {
      return DATA->count(id) ? DATA->at(id) : noobj();
    }

    List<IDxOBJ> read(const fURI_p &furi, [[maybe_unused]] const ID &source) override {
      if (furi->is_pattern()) {
        List<IDxOBJ> list;
        for (const auto &[f, o]: *this->DATA) {
          if (f->matches(*furi)) {
            list.push_back({f, o});
          }
        }
        return list;
      } else {
        const ID_p id = id_p(*furi);
        if (DATA->count(id)) {
          return {{id, DATA->at(id)}};
        } else {
          return {};
        }
      }
    }
  };
} // namespace fhatos

#endif
