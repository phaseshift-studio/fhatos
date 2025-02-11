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
#ifndef fhatos_base_mqtt_hpp
#define fhatos_base_mqtt_hpp

#include <chrono>
#include "../../../fhatos.hpp"
#include "../../structure.hpp"
#include "../../../util/obj_helper.hpp"

#ifndef FOS_MQTT_BROKER
#define FOS_MQTT_BROKER localhost
#endif

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000

namespace fhatos {
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;
  using std::chrono::system_clock;

  const static ID MQTT_FURI = ID("/sys/lib/mqtt");

  class BaseMqtt : public Structure {
  protected:
    // +[scheme]//+[authority]/#[path]
    explicit BaseMqtt(const Pattern &pattern, const ID_p &value_id = nullptr, const Rec_p &config = Obj::to_rec()) :
      Structure(pattern, id_p(MQTT_FURI), value_id, config) {
    }

    virtual bool exists() const = 0;

    virtual void native_mqtt_subscribe(const Subscription_p &subscription) = 0;

    virtual void native_mqtt_unsubscribe(const fURI_p &pattern) = 0;

    virtual void native_mqtt_publish(const Message_p &message) = 0;

    virtual void native_mqtt_disconnect() = 0;

    void connection_logging() const {
      LOG_STRUCTURE(INFO, this, "!b%s !ymqtt!! %s connected\n",
                    this->vid->toString().c_str(),
                    this->rec_get("config")->toString().c_str());
    }

  public:

    template<typename STRUCTURE>
    static void *import(const ID &import_id) {
      static_assert(std::is_base_of_v<BaseMqtt, STRUCTURE>, "STRUCTURE should be derived from BaseMqtt");
      Router::import_structure<STRUCTURE>(import_id,MQTT_FURI);
      return nullptr;
    }

    void stop() override {
      LOG_STRUCTURE(INFO, this, "!ydisconnecting!! from !g[!y%s!g]!!\n",
                    this->rec_get("config")->rec_get("broker")->toString().c_str());
      native_mqtt_disconnect();
      Structure::stop();
    }

    void recv_subscription(const Subscription_p &subscription) override {
      check_availability("subscription");
      const bool mqtt_sub = !this->has_equal_subscription_pattern(subscription->pattern());
      Structure::recv_subscription(subscription);
      if(mqtt_sub) {
        LOG_STRUCTURE(TRACE, this, "subscribing as no existing subscription found: %s\n",
                      subscription->toString().c_str());
        native_mqtt_subscribe(subscription);
      }
    }

    void recv_unsubscribe(const ID_p &source, const fURI_p &target) override {
      check_availability("unsubscribe");
      const bool mqtt_sub = this->has_equal_subscription_pattern(target);
      Structure::recv_unsubscribe(source, target);
      if(mqtt_sub && !this->has_equal_subscription_pattern(target)) {
        LOG_STRUCTURE(TRACE, this, "unsubscribing from mqtt broker as no existing subscription pattern found: %s\n",
                      target->toString().c_str());
        native_mqtt_unsubscribe(target);
      }
    }

    void loop() override {
      Structure::loop();
      ///  Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
    }

    IdObjPairs read_raw_pairs(const fURI_p &furi) override {
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const bool pattern_or_branch = furi->is_pattern() || furi->is_branch();
      const Pattern_p temp = furi->is_branch() ? p_p(furi->extend("+")) : p_p(*furi);
      auto thing = new std::atomic<List<Pair<ID_p, Obj_p>> *>(new List<Pair<ID_p, Obj_p>>());
      const auto source_id = id_p(
          this->rec_get("config/client", vri("fhatos_client"))->uri_value().toString().c_str());
      this->recv_subscription(
          Subscription::create(source_id, temp,
                               InstBuilder::build(StringHelper::cxx_f_metadata(__FILE__,__LINE__))
                               ->inst_f([thing](const Obj_p &lhs, const InstArgs &args) {
                                        thing->load()->push_back({
                                        id_p(ROUTER_READ(id_p("target"))->uri_value()), 
                                        ROUTER_READ(id_p("payload"))});
                                     return lhs;
                                   })->create()));
      ///////////////////////////////////////////////
      const milliseconds start_timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      while((duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start_timestamp) <
            milliseconds(this->rec_get("config/read_ms_wait", jnt(1000))->int_value())) {
        if(!pattern_or_branch && !thing->load()->empty())
          break;
        this->loop();
      }
      ///////////////////////////////////////////////
      this->recv_unsubscribe(id_p(source_id), temp);
      const auto list = IdObjPairs(*thing->load());
      delete thing;
      return list;
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      LOG_STRUCTURE(DEBUG, this, "!g!_writing!! %s => !b%s!! !g[!y%s!g]!!\n", obj->toString().c_str(),
                    id->toString().c_str(), retain ? "retain" : "transient");
      /*if(id == this->pattern->retract_pattern()->extend("config/connected")) {
        this->
      }*/
      native_mqtt_publish(Message::create(id, obj, retain));
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
