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
#ifndef fhatos_key_value_hpp
#define fhatos_key_value_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/structure.hpp"

namespace fhatos {

  class KeyValue : public Structure {

  protected:
    ptr<Map<ID_p, const Obj_p, furi_p_less>> DATA = share(Map<ID_p, const Obj_p, furi_p_less>());
    MutexRW<> MUTEX_DATA = MutexRW<>("<key value data>");

    explicit KeyValue(const Pattern &pattern) : Structure(pattern, SType::READWRITE) {};

  public:
    static ptr<KeyValue> create(const Pattern &pattern) {
      ptr<KeyValue> kv_p = ptr<KeyValue>(new KeyValue(pattern));
      return kv_p;
    }

    void stop() override {
      Structure::stop();
      DATA->clear();
    }

    void write(const ID_p &target, const Obj_p &payload, const ID_p &source) override {
      const Obj_p payload_copy = share(Obj(any(payload->_value), id_p(*payload->id())));
      MUTEX_DATA.write<void *>([this, target, &payload_copy, source]() {
        if (DATA->count(target)) {
          DATA->erase(target);
        }
        DATA->insert({id_p(*target), payload_copy}); // why such a deep copy needed?
        // don't forget to update subscriptions
        return nullptr;
      });
    }

    Obj_p read(const fURI_p &furi, [[maybe_unused]] const ID_p &source) override {
      if (furi->is_pattern()) {
        return MUTEX_DATA.read<Objs_p>([this, furi]() {
          Objs_p objs = Obj::to_objs();
          if (furi->is_pattern()) {
            for (const auto &[f, o]: *this->DATA) {
              if (f->matches(*furi)) {
                objs->add_obj(uri(f));
              }
            }
          } else {
            const ID_p id = id_p(*furi);
            const Obj_p toadd = DATA->count(id) ? obj(*(DATA->at(id))) : noobj();
            objs->add_obj(toadd);
          }
          return objs;
        });
      } else
        return MUTEX_DATA.read<Obj_p>(
                [this, furi]() { return DATA->count(id_p(*furi)) ? obj(*(DATA->at(id_p(*furi)))) : noobj(); });
    }
  };
} // namespace fhatos

#endif
