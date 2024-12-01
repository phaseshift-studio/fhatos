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
#include <process/util/mutex_rw.hpp>
#include <structure/structure.hpp>

#ifdef ESP_ARCH
#include <util/esp/psram_allocator.hpp>
#endif

namespace fhatos {
  template<typename ALLOCATOR = std::allocator<std::pair<const ID_p, Obj_p>>>
  class Heap final : public Structure {
  protected:
    const unique_ptr<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>> data_ =
        make_unique<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>>();
    MutexRW<> mutex_data_ = MutexRW<>("<heap_data>");

  public:
    explicit Heap(const Rec_p &structure_rec) :
      Structure(structure_rec) {
    }

    static unique_ptr<Heap> create(const Pattern &pattern, const ID &value_id = ID("")) {
      unique_ptr<Heap> heap = make_unique<Heap>(Obj::to_rec({{"pattern", vri(pattern)}}, HEAP_FURI));
      return heap;
    }

    void stop() override {
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      if (retain) {
        this->mutex_data_.template write<ID>([this, id, obj]() {
          if (this->data_->count(id))
            this->data_->erase(id);
          if (!obj->is_noobj()) {
            this->data_->insert({id_p(*id), obj->clone()});
          }
          return id;
        });
      }
      this->distribute_to_subscribers(Message::create(*id, obj->clone(), retain));
    }

    IdObjPairs read_raw_pairs(const fURI_p &match) override {
      return this->mutex_data_.template read<IdObjPairs>([this, match] {
        auto list = IdObjPairs();
        for (const auto &[id, obj]: *this->data_) {
          if (id->matches(*match)) {
            //LOG_STRUCTURE(INFO, this, "\tmatched: %s ~ %s => %s\n", id->toString().c_str(), match->toString().c_str(),
            //              obj->toString().c_str());
            list.push_back({id, obj}); // clone()?
          }
        }
        return list;
      });
    }
  };

#ifdef ESP_ARCH
  using HeapPSRAM = Heap<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
#endif
} // namespace fhatos
#endif