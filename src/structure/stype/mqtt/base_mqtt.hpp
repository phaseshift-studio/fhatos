//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#ifndef fhatos_base_mqtt_hpp
#define fhatos_base_mqtt_hpp

#include <chrono>
#include <fhatos.hpp>
#include <structure/structure.hpp>
#include FOS_PROCESS(scheduler.hpp)

#ifndef FOS_MQTT_BROKER
#define FOS_MQTT_BROKER localhost
#endif

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000

namespace fhatos {
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;
  using std::chrono::system_clock;

  class BaseMqtt : public Structure {
  public:
    struct Settings {
      string client_;
      string broker_;
      Message_p will_;
      uint16_t read_ms_wait_;
      bool connected_;

      explicit Settings(const string &client = STR(FOS_MACHINE_NAME), const string &broker = STR(FOS_MQTT_BROKER),
                        const Message_p &will = nullptr, const uint16_t read_ms_wait = 500,
                        const bool connected = true) :
        client_(client), broker_(broker), will_(will),
        read_ms_wait_(read_ms_wait), connected_(connected) {
      }
    };

  protected:
    Settings settings_;

    // +[scheme]//+[authority]/#[path]
    explicit BaseMqtt(const Pattern &pattern, const Settings &settings, const ID &value_id) :
      Structure(pattern, value_id, SType::MQTT),
      settings_(settings) {
    }

    virtual bool exists() const = 0;

    virtual void native_mqtt_subscribe(const Subscription_p &subscription) = 0;

    virtual void native_mqtt_unsubscribe(const fURI_p &pattern) = 0;

    virtual void native_mqtt_publish(const Message_p &message) = 0;

    virtual void native_mqtt_disconnect() = 0;

    void connection_logging() const {
      LOG_STRUCTURE(INFO, this,
                    "\n" FOS_TAB_4 "!ybroker address!!: !b%s!!\n" FOS_TAB_4 "!yclient name!!   : !b%s!!\n" FOS_TAB_4
                    "!ywill topic!!    : !m%s!!\n" FOS_TAB_4 "!ywill message!!  : !m%s!!\n" FOS_TAB_4
                    "!ywill qos!!      : !m%s!!\n" FOS_TAB_4 "!ywill retain!!   : !m%s!!\n",
                    this->settings_.broker_.c_str(), this->settings_.client_.c_str(),
                    this->settings_.will_.get() ? this->settings_.will_->target().toString().c_str() : "<none>",
                    this->settings_.will_.get() ? this->settings_.will_->payload()->toString().c_str() : "<none>",
                    this->settings_.will_.get() ? "1" : "<none>",
                    this->settings_.will_.get() ? FOS_BOOL_STR(this->settings_.will_->retain()) : "<none>");
    }

  public:
    void stop() override {
      LOG_STRUCTURE(INFO, this, "!ydisconnecting!! from !g[!y%s!g]!!\n",
                    this->settings_.broker_.c_str());
      native_mqtt_disconnect();
      Structure::stop();
    }

    void recv_subscription(const Subscription_p &subscription) override {
      check_availability("subscription");
      const bool mqtt_sub = !this->has_equal_subscription_pattern(furi_p(subscription->pattern()));
      Structure::recv_subscription(subscription);
      if (mqtt_sub) {
        LOG_STRUCTURE(TRACE, this, "subscribing as no existing subscription found: %s\n",
                      subscription->toString().c_str());
        native_mqtt_subscribe(subscription);
      }
    }

    void recv_unsubscribe(const ID_p &source, const fURI_p &target) override {
      check_availability("unsubscribe");
      const bool mqtt_sub = this->has_equal_subscription_pattern(target);
      Structure::recv_unsubscribe(source, target);
      if (mqtt_sub && !this->has_equal_subscription_pattern(target)) {
        LOG_STRUCTURE(TRACE, this, "unsubscribing from mqtt broker as no existing subscription pattern found: %s\n",
                      target->toString().c_str());
        native_mqtt_unsubscribe(target);
      }
    }

    void loop() override {
      Structure::loop();
      ///  Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
    }

    IdObjPairs_p read_raw_pairs(const fURI_p &furi) override {
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const bool pattern_or_branch = furi->is_pattern() || furi->is_branch();
      const fURI temp = furi->is_branch() ? furi->extend("+") : *furi;
      auto thing = new std::atomic<List<Pair<ID_p, Obj_p>> *>(new List<Pair<ID_p, Obj_p>>());
      const auto source_id = ID(this->settings_.client_.c_str());
      this->recv_subscription(
          Subscription::create(source_id, temp, Obj::to_bcode(
                                   [this, furi, thing](const Obj_p &, const InstArgs &args) {
                                     const Message_p message = make_shared<Message>(args.at(0));
                                     LOG_STRUCTURE(DEBUG, this, "subscription pattern %s matched: %s\n",
                                                   furi->toString().c_str(),
                                                   message->toString().c_str());
                                     ///Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
                                     thing->load()->push_back({id_p(message->rec_get(vri("target"))->uri_value()),
                                                               message->rec_get(vri("payload"))});
                                     return noobj();
                                   }, {x(0)}, StringHelper::cxx_f_metadata(__FILE__,__LINE__))));
      ///////////////////////////////////////////////
      const milliseconds start_timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
      while ((duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start_timestamp) <
             milliseconds(this->settings_.read_ms_wait_)) {
        if (!pattern_or_branch && !thing->load()->empty())
          break;
        this->loop();
      }
      ///////////////////////////////////////////////
      this->recv_unsubscribe(id_p(source_id), furi_p(temp));
      const IdObjPairs_p list = ptr<IdObjPairs>(thing->load());
      delete thing;
      return list;
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      LOG_STRUCTURE(DEBUG, this, "!g!_writing!! %s => !b%s!! !g[!y%s!g]!!\n", obj->toString().c_str(),
                    id->toString().c_str(), retain ? "retain" : "transient");
      /*if(id == this->pattern()->retract_pattern()->extend("config/connected")) {
        this->
      }*/
      native_mqtt_publish(Message::create(*id, obj, retain));
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