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

#include "../../fhatos.hpp"
#include <shared_mutex>
#include "../../language/obj.hpp"
#include "../structure.hpp"

#ifdef ESP_ARCH
#include "../../util/esp/psram_allocator.hpp"
#endif

namespace fhatos {
  template<typename ALLOCATOR = std::allocator<std::pair<const ID_p, Obj_p>>>
  class Heap final : public Structure {
  protected:
    const unique_ptr<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>> data_ =
        make_unique<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>>();
    std::shared_mutex map_mutex;

  public:
    explicit Heap(const Rec_p &structure_rec) : Structure(structure_rec) {
    }

    static unique_ptr<Heap> create(const Pattern &pattern) {
      unique_ptr<Heap> heap = make_unique<Heap>(Obj::to_rec({{"pattern", vri(pattern)}}, HEAP_FURI));
      return heap;
    }

    void stop() override {
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      if(retain) {
        std::lock_guard<std::shared_mutex> lock(this->map_mutex);
        if(obj->is_noobj())this->data_->erase(id);
        else this->data_->insert_or_assign(id, obj);
      }
      this->distribute_to_subscribers(Message::create(*id, obj, retain));
    }

    IdObjPairs read_raw_pairs(const fURI_p &match) override {
      std::shared_lock<std::shared_mutex> lock(this->map_mutex);
      auto list = IdObjPairs();
      for(const auto &[id, obj]: *this->data_) {
        if(id->matches(*match)) {
          list.push_back({id, obj});
        }
      }
      return list;
    }
  };

#ifdef ESP_ARCH
  using HeapPSRAM = Heap<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
#endif
} // namespace fhatos
#endif
