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
#ifndef fhatos_heaped_hpp
#define fhatos_heaped_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/structure.hpp>
#include <process/util/mutex_rw.hpp>

namespace fhatos {
  class Heap : public Structure {
  public:
    Map_p<ID_p, const Obj_p, furi_p_less> data_ = make_shared<Map<ID_p, const Obj_p, furi_p_less>>();
    MutexRW<> mutex_data_ = MutexRW<>("<heap_data>");

    explicit Heap(const Pattern &pattern, const SType stype = SType::HEAP) : Structure(pattern, stype) {
    }

  public:
    static ptr<Heap> create(const Pattern &pattern) {
      auto heap_p = ptr<Heap>(new Heap(pattern));
      return heap_p;
    }

    void stop() override {
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      if (retain) {
        this->mutex_data_.write<ID>([this,id,obj]() {
          if (this->data_->count(id))
            this->data_->erase(id);
          if (!obj->is_noobj()) {
            this->data_->insert({id_p(*id), obj});
          }
          return id;
        });
      }
      this->distribute_to_subscribers(message_p(*id, obj->clone(), retain));
    }

    IdObjPairs_p read_raw_pairs(const fURI_p &match) override {
      return this->mutex_data_.read<IdObjPairs_p>([this,match] {
        auto list = make_shared<IdObjPairs>();
        for (const auto &[id, obj]: *this->data_) {
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
