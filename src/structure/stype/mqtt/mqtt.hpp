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
#include "../../../util/obj_helper.hpp"
#include "../../../lang/obj.hpp"
#include "../../router.hpp"

#ifndef FOS_MQTT_BROKER
#define FOS_MQTT_BROKER localhost
#endif

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000
#ifdef NATIVE
#define FOS_MQTT_WAIT_TIME 250
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

    // +[scheme]//+[authority]/#[path]
    explicit Mqtt(const Pattern &pattern, const ID_p &value_id = nullptr,
                  const Rec_p &config = Obj::to_rec());

    ~Mqtt() override = default;

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
      native_mqtt_disconnect();
      Structure::stop();
    }

    void recv_subscription(const Subscription_p &subscription) override {
      check_availability("subscription");
      const bool mqtt_sub = !this->has_equal_subscription_pattern(*subscription->pattern());
      Structure::recv_subscription(subscription);
      if(mqtt_sub) {
        LOG_WRITE(TRACE, this, L("subscribing as no existing subscription found: {}\n",
                                 subscription->toString()));
        native_mqtt_subscribe(subscription);
      }
    }

    void recv_unsubscribe(const ID &source, const fURI &target) override {
      check_availability("unsubscribe");
      const bool mqtt_sub = this->has_equal_subscription_pattern(target);
      Structure::recv_unsubscribe(source, target);
      if(mqtt_sub && !this->has_equal_subscription_pattern(target)) {
        LOG_WRITE(TRACE, this, L("unsubscribing from mqtt broker as no existing subscription pattern found: {}\n",
                                 target.toString()));
        native_mqtt_unsubscribe(target);
      }
    }

    static void *import(const ID &import_id) {
      Router::import_structure<Mqtt>(import_id, MQTT_FURI);
      return nullptr;
    }

    static ptr<Mqtt> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                            const Rec_p &config = Obj::to_rec()) {
      return Structure::create<Mqtt>(pattern, value_id, config);
    }

    IdObjPairs read_raw_pairs(const fURI &furi) override {
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const bool pattern_or_branch = furi.is_pattern() || furi.is_branch();
      const fURI temp = furi.is_branch() ? furi.extend("+") : furi;
      const fURI temp_2 = temp.has_wildcard() ? temp.retract_pattern().extend("#") : temp;
      auto thing = std::make_unique<std::vector<std::pair<ID, Obj_p>>>();
      std::vector<std::pair<ID, Obj_p>> *thing_p = thing.get();
      const auto source_id = id_p(this->rec_get("config/client")
          ->or_else(vri("fhatos_client"))->uri_value().toString().c_str());
      this->recv_subscription(
          Subscription::create(source_id, p_p(temp_2),
                               InstBuilder::build(StringHelper::cxx_f_metadata(__FILE__,__LINE__))
                               ->inst_f([thing_p](const Obj_p &lhs, const InstArgs &) {
                                 //LOG(INFO,"TARGET/PAYLOAD/RETAIN: %s %s %s\n",ROUTER_READ("target")->toString().c_str(),ROUTER_READ("payload")->toString().c_str(),ROUTER_READ("retain")->toString().c_str());
                                 //LOG(INFO,"LHS/ARGS:              %s %s\n",lhs->toString().c_str(),args->toString().c_str());
                                 const Obj_p o = ROUTER_READ("target");
                                 if(nullptr == o || !o->value_.has_value())
                                   LOG_WRITE(ERROR, o.get(), L("!ytarget uri !rcorrupted!! at mqtt broker"));
                                 else
                                   thing_p->emplace_back(ID(o->uri_value()), lhs);
                                 return lhs;
                               })->create()));
      ///////////////////////////////////////////////
      const milliseconds start_timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      while((duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start_timestamp) <
            milliseconds(this->rec_get("config/read_ms_wait")->or_else(jnt(FOS_MQTT_WAIT_TIME))->int_value())) {
        if(!pattern_or_branch && !thing->empty())
          break;
        FEED_WATCHDOG();
        this->loop();
      }
      ///////////////////////////////////////////////
      this->recv_unsubscribe(*source_id, temp);
      return std::vector<std::pair<ID, Obj_p>>(*thing);
    }

    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      LOG_WRITE(DEBUG, this, L("!g!_writing!! {} => !b{}!! !g[!y{}!g]!!\n", obj->toString(),
                               id.toString(), retain ? "retain" : "transient"));
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
