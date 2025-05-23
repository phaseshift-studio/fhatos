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

#include <shared_mutex>
#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../sys/router/router.hpp"
#include "../sys/router/structure.hpp"
#include "../sys/scheduler/thread/mutex.hpp"

#ifdef ESP_PLATFORM
#include "../../../util/esp32/psram_allocator.hpp"
#endif

#define HEAP_TID  "/fos/s/heap"

namespace fhatos {
#ifdef ESP_PLATFORM
  template <typename ALLOCATOR = PSRAMAllocator<std::pair<const ID_p, Obj_p>>>
#else
  template<typename ALLOCATOR = std::allocator<std::pair<const ID, Obj_p>>>
#endif
  class Heap final : public Structure {
  protected:
    const unique_ptr<Map<const ID, Obj_p, furi_less, ALLOCATOR>> data_ =
        make_unique<Map<const ID, Obj_p, furi_less, ALLOCATOR>>();
    Mutex map_mutex;

  public:
    explicit Heap(const Pattern &span, const ID_p &vid = nullptr, const Rec_p &config = Obj::to_rec()) :
        Structure(span, id_p(HEAP_TID), vid, config) {}

    static Structure_p create(const Pattern &span, const ID_p &vid = nullptr,
                              const Rec_p &config = Obj::to_rec()) {
      return Structure::create<Heap<ALLOCATOR>>(span, vid, config);
    }

    static void register_module() {
      Router::register_structure_module<Heap>(HEAP_TID);
    }

    void stop() override {
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      if(retain) {
        auto lock = std::lock_guard<Mutex>(this->map_mutex);
        if(obj->is_noobj()) {
          if(this->data_->count(id))
            this->data_->erase(id);
        } else {
          this->data_->insert_or_assign(ID(id), obj->clone()); // clone?
        }
      }
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      auto list = IdObjPairs();
      auto lock = std::shared_lock<Mutex>(this->map_mutex);
      if(!match.is_pattern()) {
        if(this->data_->count(match))
          list.push_back(std::make_pair<const ID, Obj_p>(ID(match), this->data_->at(match)->clone()));
      } else {
        for(const auto &[id, obj]: *this->data_) {
          if(id.matches(match)) {
            list.push_back(std::make_pair<const ID, Obj_p>(ID(id), obj->clone())); // TODO: clone?
          }
        }
      }
      return list;
    }

    bool has(const fURI &furi) override {
      auto lock = std::shared_lock<Mutex>(this->map_mutex);
      if(!furi.is_pattern() && this->data_->count(furi))
        return true;
      for(const auto &[id, obj]: *this->data_) {
        if(id.matches(furi)) {
          return true;
        }
      }
      return false;
    }
  };

  #ifdef ESP_PLATFORM
    using HeapPSRAM = Heap<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
  #endif
} // namespace fhatos
#endif
