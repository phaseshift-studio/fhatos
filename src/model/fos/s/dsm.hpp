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

#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../../structure/util/mqtt/mqtt_client.hpp"
#include "../../../util/mutex_map.hpp"
#include "../sys/router/router.hpp"
#include "../sys/router/structure.hpp"

/*
#ifdef ESP_PLATFORM
#include "../../util/esp32/psram_allocator.hpp"
#endif*/

#define MQTT_WAIT_MS 250
#define DSM_TID "/fos/s/dsm"

namespace fhatos {

  // template<typename ALLOCATOR = std::allocator<std::pair<const ID, Obj_p>>>
  class DSM final : public Structure {
  protected:
    const uptr<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>> data_ =
        make_unique<MutexMap<const ID, Obj_p, std::less<>, std::allocator<std::pair<const ID, Obj_p>>>>();
    size_t cache_size_ = 100;
    bool async = false;
    ptr<MqttClient> mqtt{};

    [[nodiscard]] Subscription_p generate_sync_subscription(const Pattern &pattern) const {
      return Subscription::create(this->vid, p_p(pattern), [this](const Obj_p &obj, const InstArgs &args) {
        this->write_raw_raw_pairs(args->arg("target")->uri_value(), obj, args->arg("retain")->bool_value());
        return Obj::to_noobj();
      });
    }

    void clear_cache(const size_t new_size = 0, const Pattern & = "#") const {
      size_t to_delete = this->data_->size() - new_size;
      while(to_delete-- > 0) {
        if(this->data_->empty())
          break;
        this->data_->pop();
      }
    }

  public:
    explicit DSM(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
        Structure(pattern, id_p(DSM_TID), value_id, config), mqtt{nullptr} {
      this->cache_size_ = config->rec_get("cache_size")->or_else_(100);
      this->async = this->rec_get("config/async")->or_else_(false);
    }

    static Structure_p create(const Pattern &pattern, const ID_p &value_id = nullptr,
                              const Rec_p &config = Obj::to_rec({{"async", __().else_(dool(true))},
                                                                 {"broker", __().else_(vri("mqtt://localhost:1883"))},
                                                                 {"client", __().else_(vri("fhatos_client"))}})) {
      return Structure::create<DSM>(pattern, value_id, config);
    }

    static void *register_module() {
      Router::register_structure_module<DSM>(DSM_TID);
      return nullptr;
    }

    void loop() override {
      Structure::loop();
      this->mqtt->loop();
    }

    void setup() override {
      const auto q_sub = static_cast<const QSub *>(this->q_procs_->rec_get("sub").get());
      this->mqtt = MqttClient::get_or_create(this->get<fURI>("config/broker"), this->get<fURI>("config/client"));
      const_cast<QSub *>(q_sub)->set_post<MqttClient>([this]() {
        return MqttClient::get_or_create(this->obj_get("config/broker")->uri_value(),
                                         this->obj_get("config/client")->uri_value());
      });
      if(this->cache_size_ > 0) {
        this->mqtt->on_connect = [this]() {
          LOG_WRITE(INFO, this, L("!ystructure pattern !b{}!! subscribed\n", this->pattern->toString()));
          this->mqtt->subscribe(generate_sync_subscription(*this->pattern), true);
        };
      }
      if(!this->mqtt->connect(*this->vid))
        LOG_WRITE(WARN, this, L("!runable!y to connect to !b{}!!\n", this->get<fURI>("config/broker").toString()));
      Structure::setup();
    }

    void stop() override {
      assert(this->mqtt->disconnect(*this->vid, false));
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
      // this->mqtt->publish(Message::create(id_p(id), obj, retain), this->async);
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      if(!match.is_pattern() && match.is_node()) {
        if(this->data_->exists(match)) {
          return {{match, this->data_->at(match)}};
        } else {
          if(-1 != this->cache_size_ && this->data_->size() >= this->cache_size_)
            this->clear_cache(this->data_->size() - 1);
          if(!this->pattern->equals(match))
            this->mqtt->subscribe(this->generate_sync_subscription(match), this->async);
          // this->loop();
          // Thread::delay_current_thread(MQTT_WAIT_MS);
          this->loop();
          if(!this->pattern->equals(match))
            this->mqtt->unsubscribe(*this->vid, match, this->async);
          // this->mqtt->on_connect();
          if(this->data_->exists(match)) {
            const IdObjPairs pairs = {{match, this->data_->at(match)}};
            return pairs;
          }
        }
        return {};
      }
      //////////////////////////////////////////////////////////////////
      auto list = IdObjPairs();
      for(const auto &[id, obj]: *this->data_) {
        if(id.matches(match)) {
          list.emplace_back(id, obj);
        }
      }
      return list;
    }

    bool has(const fURI &furi) override {
      if(!furi.is_pattern() && furi.is_node())
        return this->data_->count(furi) > 0;
      return std::any_of(this->data_->begin(), this->data_->end(),
                         [&furi](const auto &pair) { return pair.first.matches(furi); });
    }
  };

  /*#ifdef ESP_PLATFORM
    using DSM_PSRAM = DSM<PSRAMAllocator<std::pair<const ID_p, Obj_p>>>;
  #endif*/
} // namespace fhatos
#endif
