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
#include "../../lang/obj.hpp"
#include "../structure.hpp"
#include "../router.hpp"
#include "../../util/mutex_map.hpp"
#include "../util/mqtt/mqtt_client.hpp"

#ifdef ESP_PLATFORM
#include "../../util/esp32/psram_allocator.hpp"
#endif

#define MQTT_WAIT_MS 250

namespace fhatos {
  const static auto DSM_FURI = ID("/sys/lib/dsm");
  const static Rec_p DSM_DEFAULT_CONFIG =
      Obj::to_rec({{"async", __().else_(dool(true))},
                   {"broker", __().else_(vri("mqtt://localhost:1883"))},
                   {"client", __().else_(vri("fhatos_client"))}});

  template<typename ALLOCATOR = std::allocator<std::pair<const ID, Obj_p>>>
  class DSM final : public Structure {
  protected:
    const uptr<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>> data_ =
        make_unique<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>>();
    int cache_size_ = 100;
    ptr<MqttClient> mqtt{};

    [[nodiscard]] Subscription_p generate_sync_subscription(const Pattern &pattern) const {
      return Subscription::create(this->vid, p_p(pattern), [this](const Obj_p &obj, const InstArgs &args) {
        this->write_raw_raw_pairs(
            args->arg("target")->uri_value(), obj,
            args->arg("retain")->bool_value());
        return Obj::to_noobj();
      });
    }

    void clear_cache(const int new_size = 0, const Pattern & = "#") const {
      int to_delete = this->data_->size() - new_size;
      while(to_delete-- > 0) {
        this->data_->pop();
      }
    }

    void wait_for_data(const int milliseconds) {
      this->loop();
      if(const Option<Thread *> op = Thread::current_thread(); op.has_value()) {
        op.value()->delay(milliseconds);
      }
      this->loop();
    }

  public:
    explicit DSM(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
      Structure(pattern, id_p(DSM_FURI), value_id, config),
      mqtt{nullptr} {
      this->cache_size_ = config->rec_get("cache_size", jnt(100))->int_value();
      // this->Obj::rec_set("config",config->rec_merge(Router::singleton()->rec_get("config/default_config")->clone()->rec_value()));
    }

    static Structure_p create(const Pattern &pattern, const ID_p &value_id = nullptr,
                              const Rec_p &config = DSM_DEFAULT_CONFIG) {
      return Structure::create<DSM>(pattern, value_id, config);
    }

    static void *import(const ID &import_id) {
      Router::import_structure<DSM>(import_id, DSM_FURI);
      return nullptr;
    }

    void loop() override {
      Structure::loop();
      this->mqtt->loop();
    }

    void setup() override {
      this->mqtt = MqttClient::get_or_create(this->get<fURI>("config/broker"),
                                             this->get<fURI>("config/client"));

      if(this->mqtt->connect(*this->vid))
        this->mqtt->subscribe(generate_sync_subscription(*this->pattern), this->get<bool>("config/async"));
      Structure::setup();
    }

    void stop() override {
      this->mqtt->disconnect(*this->vid, this->get<bool>("config/async"));
      Structure::stop();
      this->data_->clear();
    }

  protected:
    void write_raw_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) const {
      if(retain) {
        if(obj->is_noobj()) {
          if(this->data_->exists(id))
            this->data_->erase(id);
        } else {
          if(-1 != this->cache_size_ && this->data_->size() >= this->cache_size_)
            this->clear_cache(this->data_->size() - 1);
          this->data_->insert_or_assign(id, const_pointer_cast<Obj>(obj));
        }
      }
    }


    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      this->write_raw_raw_pairs(id, obj, retain);
      this->mqtt->publish(Message::create(id_p(id), obj, retain), this->get<bool>("config/async"));
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      if(!match.is_pattern() && match.is_node()) {
        if(this->data_->exists(match)) {
          return {{match, this->data_->at(match)}};
        } else {
          if(-1 != this->cache_size_)
            this->clear_cache(this->cache_size_ - 1);
          this->mqtt->subscribe(this->generate_sync_subscription(match), this->get<bool>("config/async"));
          this->wait_for_data(MQTT_WAIT_MS);
          this->loop();
          this->mqtt->unsubscribe(*this->vid, match, this->get<bool>("config/async"));
          if(this->data_->exists(match)) {
            const IdObjPairs pairs = {{match, this->data_->at(match)}};
            return pairs;
          }
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

    bool has(const fURI &furi) override {
      if(!furi.is_pattern() && furi.is_node())
        return this->data_->count(furi) > 0;
      for(const auto &[id, obj]: *this->data_) {
        if(id.matches(furi)) {
          return true;
        }
      }
      return false;
    }
  };

#ifdef ESP_PLATFORM
  using DSM_PSRAM = DSM<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
#endif
} // namespace fhatos
#endif
