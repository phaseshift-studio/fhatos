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
    Map<ID_p, const Obj_p, furi_p_less> data_ = Map<ID_p, const Obj_p, furi_p_less>();
    MutexRW<> mutex_data_ = MutexRW<>("<key value data>");

    explicit KeyValue(const Pattern &pattern, const SType stype = SType::LOCAL) : Structure(pattern, stype) {
    };

  public:
    static ptr<KeyValue> create(const Pattern &pattern) {
      auto kv_p = ptr<KeyValue>(new KeyValue(pattern));
      return kv_p;
    }

    void stop() override {
      Structure::stop();
      data_.clear();
    }

  protected:
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      if (retain) {
        this->mutex_data_.write<ID>([this,id,obj]() {
          if (this->data_.count(id))
            this->data_.erase(id);
          if (!obj->is_noobj()) {
            this->data_.insert({id, obj->clone()});
          }
          return id;
        });
      }
      this->distribute_to_subscribers(message_p(*id, obj, retain));
    }

    List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &match) override {
      return *this->mutex_data_.read<List_p<Pair<ID_p, Obj_p>>>([this,match] {
        auto list = make_shared<List<Pair<ID_p, Obj_p>>>();
        for (const auto &[id, obj]: this->data_) {
          if (id->matches(*match)) {
            list->push_back({id, obj});
          }
        }
        return list;
      });
    }
  };
} // namespace fhatos

#endif
