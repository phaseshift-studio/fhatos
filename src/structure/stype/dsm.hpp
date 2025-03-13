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
#ifndef fhatos_dsm_hpp
#define fhatos_dsm_hpp

#include "../../fhatos.hpp"
#include <shared_mutex>
#include "../../lang/obj.hpp"
#include "../structure.hpp"
#include "../router.hpp"
#include "../../util/mutex_map.hpp"
#include "../util/mqtt/mqtt_client.hpp"
#include "../../model/fos/sys/thread/fmutex.hpp"

#ifdef ESP_ARCH
#include "../../util/esp32/psram_allocator.hpp"
#endif

namespace fhatos {
  const static ID DSM_FURI = ID("/sys/lib/dsm");

  /*template<typename ALLOCATOR = std::allocator<std::pair<const ID, Obj_p>>>*/
  class DSM final : public Structure {
  protected:
    const uptr<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>> data_ =
        make_unique<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>>();
    int max_size = 100;
    unique_ptr<MqttClient> mqtt;

  public:
    explicit DSM(const Pattern &pattern, const ID_p &value_id = nullptr,
                 const Rec_p &config = Obj::to_rec()) :
      Structure(pattern, id_p(DSM_FURI), value_id, config),
      mqtt(nullptr) {
      this->max_size = config->rec_get("max_size", jnt(1000))->int_value();
      // this->Obj::rec_set("config",config->rec_merge(Router::singleton()->rec_get("config/default_config")->clone()->rec_value()));
    }

    static Structure_p create(const Pattern &pattern, const ID_p &value_id = nullptr,
                              const Rec_p &config = Obj::to_rec()) {
      const Structure_p s = Structure::create<DSM>(pattern, value_id, config);
      return s;
    }

    static void *import(const ID &import_id) {
      Router::import_structure<DSM>(import_id, DSM_FURI);
      return nullptr;
    }

    void setup() override {
      this->mqtt = std::make_unique<MqttClient>(this->rec_get("config"));
      if(this->mqtt->connect(*this->vid)) {
        this->mqtt->subscribe(Subscription::create(this->vid, this->pattern,
                                                   [this](const Obj_p &obj, const InstArgs &args) {
                                                     this->write_raw_raw_pairs(
                                                         args->arg("target")->uri_value(), obj,
                                                         args->arg("retain")->bool_value());
                                                     return Obj::to_noobj();
                                                   }));
      }
      Structure::setup();
    }

    void stop() override {
      this->mqtt->disconnect(*this->vid);
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) {
      if(retain) {
        if(obj->is_noobj()) {
          if(this->data_->exists(id))
            this->data_->erase(id);
        } else {
          this->data_->insert_or_assign(id, std::move(obj));
          while(this->data_->size() >= this->max_size) {
            this->data_->pop();
          }
        }
      }
    }


    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      this->write_raw_raw_pairs(id, obj, retain);
      this->mqtt->publish(Message::create(id_p(id), obj, retain));
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      if(!match.is_pattern() && match.is_node()) {
        if(this->data_->exists(match)) {
          return {{match, this->data_->at(match)}};
        }
        return {};
      } else {
        auto list = IdObjPairs();
        for(const auto &[id, obj]: *this->data_) {
          if(id.matches(match)) {
            list.emplace_back(id, obj);
          }
        }
        return list;
      }
    }

    /*bool has(const fURI &furi) override {
      std::shared_lock<fMutex> lock(this->map_mutex);
      if(!furi.is_pattern() && this->data_->count(furi))
        return true;
      for(const auto &[id, obj]: *this->data_) {
        if(id.matches(furi)) {
          return true;
        }
      }
      return false;
    }*/
  };

#ifdef ESP_ARCH
  using DSM_PSRAM = DSM<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
#endif
} // namespace fhatos
#endif
