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
#include "../../lang/obj.hpp"
#include "../structure.hpp"

#ifdef ESP_ARCH
#include "../../util/esp32/psram_allocator.hpp"
#endif

namespace fhatos {
  const static ID HEAP_FURI = ID("/sys/lib/heap");

  template<typename ALLOCATOR = std::allocator<std::pair<const ID, Obj_p>>>
  class Heap final : public Structure {
  protected:
    const unique_ptr<Map<const ID, Obj_p, furi_less, ALLOCATOR>> data_ =
        make_unique<Map<const ID, Obj_p, furi_less, ALLOCATOR>>();
    std::shared_mutex map_mutex;

  public:
    explicit Heap(const Pattern &pattern, const ID_p &value_id = nullptr,
                  const Rec_p &config = Obj::to_rec()) :
      Structure(pattern, id_p(HEAP_FURI), value_id, config) {
      // this->Obj::rec_set("config",config->rec_merge(Router::singleton()->rec_get("config/default_config")->clone()->rec_value()));
    }

    static ptr<Heap<ALLOCATOR>> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                                       const Rec_p &config = Obj::to_rec()) {
      return Structure::create<Heap<ALLOCATOR>>(pattern, value_id, config);
    }

    static void *import(const ID &import_id) {
      Router::import_structure<Heap>(import_id, HEAP_FURI);
      return nullptr;
    }

    void stop() override {
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      Obj_p send_obj = Obj::to_noobj();
      if(retain) {
        auto lock = std::lock_guard<std::shared_mutex>(this->map_mutex);
        if(obj->is_noobj())
          this->data_->erase(id);
        else
          this->data_->insert_or_assign(std::move(ID(id)), obj);

        send_obj = obj;
      } else {
        auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
        const Obj_p eval_obj = this->data_->count(id) ? this->data_->at(id) : Obj::to_noobj();
        lock.unlock();
        send_obj = eval_obj->apply(obj);
        /*LOG(INFO, "[%s] %s => %s = %s\n", id->toString().c_str(), obj->toString().c_str(), eval_obj->toString().c_str(),
            send_obj->toString().c_str());*/
      }
      this->distribute_to_subscribers(Message::create(id_p(id), send_obj, retain));
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {

      auto list = IdObjPairs();
      if(!match.is_pattern()) {
       auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
        if(const ID id_match = ID(match); this->data_->count(id_match))
          list.push_back({id_match, this->data_->at(id_match)});
        lock.unlock();
      } else {
        auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
        for(const auto &[id, obj]: *this->data_) {
          if(id.matches(match)) {
            list.push_back({id, obj});
          }
        }
        lock.unlock();
      }
      return list;
    }

    bool has(const fURI &furi) override {
      std::shared_lock<std::shared_mutex> lock(this->map_mutex);
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

#ifdef ESP_ARCH
  using HeapPSRAM = Heap<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
#endif
} // namespace fhatos
#endif
