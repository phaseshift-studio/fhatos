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
#ifndef fhatos_mqtt_hpp
#define fhatos_mqtt_hpp

#include <chrono>
#include "../../../fhatos.hpp"
#include "../../structure.hpp"
#include "../../../lang/obj.hpp"
#include "../../router.hpp"

#ifndef FOS_MQTT_BROKER
#define FOS_MQTT_BROKER localhost
#endif

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000
#ifdef NATIVE
#define FOS_MQTT_RETRY_WAIT 2000
#else
#define FOS_MQTT_WAIT_TIME 1250
#endif

namespace fhatos {
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;
  using std::chrono::system_clock;

  const static ID MQTT_FURI = ID("/sys/lib/mqtt");

  class Mqtt final : public Structure {
  public:
    static std::vector<Mqtt *> MQTT_VIRTUAL_CLIENTS;
    std::any handler_;
    uptr<Map<ID, Obj_p>> cache_ = nullptr;
    Mutex cache_mutex_;

    // +[scheme]//+[authority]/#[path]
    explicit Mqtt(const Pattern &pattern, const ID_p &value_id = nullptr,
                  const Rec_p &config = Obj::to_rec());

    ~Mqtt() override = default;

    void disable_cache() {
      // TODO: need source on write Router::singleton()->unsubscribe(this->vid->extend("cache"), *this->pattern);
      auto lock = std::lock_guard<Mutex>(this->cache_mutex_);
      this->cache_->clear();
      this->cache_ = nullptr;
      LOG_WRITE(INFO, this,L("!ycache!! disabled\n"));
    }

    void enable_cache() {
      if(!this->cache_) {
        this->cache_ = make_unique<Map<ID, Obj_p>>();
        this->native_mqtt_subscribe(Subscription::create(this->vid, this->pattern,
                                                         [this](const Obj_p &obj, const InstArgs &args) {
                                                           this->write_cache(args->arg("target")->uri_value(), obj);
                                                           return Obj::to_noobj();
                                                         }));
        LOG_WRITE(INFO, this,L("!ycache!! enabled\n"));
      } else {
        LOG_WRITE(WARN, this, L("!ycache already!! enabled\n"));
      }
    }

    bool exists() const;

    void native_mqtt_subscribe(const Subscription_p &subscription);

    void native_mqtt_unsubscribe(const fURI &pattern);

    void native_mqtt_publish(const Message_p &message);

    void native_mqtt_disconnect();

    void connection_logging() const {
      LOG_WRITE(INFO, this, L("!b{} !ymqtt!! {} connected\n", this->vid->toString(),
                              this->rec_get("config")->toString()));
    }

    void loop() override;

    void setup() override;

    void stop() override {
      LOG_WRITE(INFO, this, L("!ydisconnecting!! from !g[!y{}!g]!!\n",
                              this->rec_get("config")->rec_get("broker")->toString()));
      Structure::stop();
      native_mqtt_disconnect();
    }

    static void *import(const ID &import_id) {
      Router::import_structure<Mqtt>(import_id, MQTT_FURI);
      return nullptr;
    }

    static ptr<Mqtt> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                            const Rec_p &config = Obj::to_rec()) {
      return Structure::create<Mqtt>(pattern, value_id, config);
    }

    IdObjPairs read_cache(const fURI &furi) {
      if(this->cache_) {
        auto lock = std::shared_lock<Mutex>(this->cache_mutex_);
        std::vector<std::pair<ID, Obj_p>> pairs;
        if(!furi.is_pattern() && furi.is_node() && (this->cache_->count(furi) > 0)) {
          pairs.emplace_back(furi, this->cache_->at(furi));
          return pairs;
        } else {
          for(const auto &[k,v]: *this->cache_) {
            if(k.bimatches(furi))
              pairs.emplace_back(k, v);
          }
          return pairs;
        }
      }
      return {};
    }

    void write_cache(const ID &id, const Obj_p &obj) {
      if(this->cache_) {
        auto lock = std::lock_guard<Mutex>(this->cache_mutex_);
        if(obj->is_noobj()) {
          if(this->cache_->count(id))
            this->cache_->erase(id);
        } else
          this->cache_->insert_or_assign(id, obj);
      }
    }

    IdObjPairs read_raw_pairs(const fURI &furi) override {
      const IdObjPairs pairs = this->read_cache(furi);
      return pairs;
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //const bool pattern_or_branch = furi.is_pattern() || furi.is_branch();
      /* const fURI temp = furi.is_branch() ? furi.extend("+") : furi;
       // const fURI temp_2 = temp.has_wildcard() ? temp.retract_pattern().extend("#") : temp;
       const auto source_id = id_p(this->rec_get("config/client")
           ->or_else(vri("fhatos_client"))->uri_value().toString().c_str());
       //const Subscription_p subscription = Subscription::create(source_id, p_p(temp), Obj::to_noobj());
       //this->native_mqtt_subscribe(subscription);
       ///////////////////////////////////////////////
       const milliseconds start_timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
       while((duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start_timestamp) <
             milliseconds(this->rec_get("config/read_ms_wait")->or_else(jnt(FOS_MQTT_WAIT_TIME))->int_value())) {
         FEED_WATCHDOG();
         this->loop();
       }
       //this->native_mqtt_unsubscribe(temp);
       return this->read_cache(furi);*/
    }

    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      LOG_WRITE(DEBUG, this, L("!g!_writing!! {} => !b{}!! !g[!y{}!g]!!\n", obj->toString(),
                               id.toString(), retain ? "retain" : "transient"));
      if(retain)
        this->write_cache(id, obj);
      native_mqtt_publish(Message::create(id_p(id), obj->clone(), retain));
    }

    void publish_retained(const Subscription_p &) override {
      // handled by mqtt broker
    }

    static BObj_p make_bobj(const Obj_p &payload, const bool retain) {
      const Lst_p lst = Obj::to_lst({payload, dool(retain)});
      return lst->serialize();
    }

    static Pair<Obj_p, bool> make_payload(const BObj_p &bobj) {
      const Lst_p lst = Obj::deserialize(bobj);
      return {lst->lst_value()->at(0), lst->lst_value()->at(1)->bool_value()};
    }
  };
} // namespace fhatos
#endif
