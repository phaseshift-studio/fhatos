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
#ifndef fhatos_space_hpp
#define fhatos_space_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/rooter.hpp"
#include "structure/structure.hpp"

namespace fhatos {

  class KeyValue : public Structure {

  protected:
    Map<fURI, const Obj_p> *DATA = new Map<fURI, const Obj_p>();
    MutexRW<> MUTEX_DATA = MutexRW<>();

    explicit KeyValue(const Pattern &pattern) : Structure(pattern, SType::READWRITE){};

  public:
    static KeyValue *create(const Pattern &pattern) { return new KeyValue(pattern); }

    ~KeyValue() override { delete DATA; }

    void stop() override {
      Structure::stop();
      DATA->clear();
    }

    void write(const fURI &target, const Obj_p &payload, const ID &source) override {
      MUTEX_DATA.write<void *>([this, target, payload, source]() {
        if (DATA->count(target)) {
          DATA->erase(target);
        }
        DATA->insert({target, payload});
        return nullptr;
      });
    }

    List<Pair<fURI_p, Obj_p>> read(const fURI &id, [[maybe_unused]] const ID &source) override {
      if (false) { // id.is_pattern()) {
        return {};
      } else if (DATA->count(id)) {
        return {Pair<fURI_p, Obj_p>({share<fURI>(id), DATA->at(id)})};
      }
      return {};
    }
  };
} // namespace fhatos

#endif
