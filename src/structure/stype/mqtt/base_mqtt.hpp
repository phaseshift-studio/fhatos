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

#include <fhatos.hpp>
#include <structure/structure.hpp>

#ifndef FOS_MQTT_BROKER_ADDR
#define FOS_MQTT_BROKER_ADDR localhost
#endif
#ifndef FOS_MQTT_BROKER_PORT
#define FOS_MQTT_BROKER_PORT 1883
#endif

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000

namespace fhatos {
  class BaseMqtt : public Structure {
  protected:
    string server_addr_;
    const Message_p will_message_;

    // +[scheme]//+[authority]/#[path]
    explicit BaseMqtt(const Pattern &pattern = Pattern("//+/#"), const string &server_addr = STR(FOS_MQTT_BROKER_ADDR),
                      const Message_p &will_message = ptr<Message>(nullptr)) : Structure(pattern, SType::NETWORK),
                                                                               server_addr_(server_addr),
                                                                               will_message_(will_message) {
    }

    virtual void native_mqtt_subscribe(const Subscription_p &subscription) = 0;

    virtual void native_mqtt_unsubscribe(const fURI_p &pattern) = 0;

    virtual void native_mqtt_publish(const Message_p &message) = 0;

    virtual void native_mqtt_disconnect() = 0;

    virtual void native_mqtt_loop() = 0;

    void connection_logging(const ID_p &client_id) const {
      LOG_STRUCTURE(INFO, this,
                    "\n" FOS_TAB_4 "!ybroker address!!: !b%s!!\n" FOS_TAB_4 "!yclient name!!   : !b%s!!\n" FOS_TAB_4
                    "!ywill topic!!    : !m%s!!\n" FOS_TAB_4 "!ywill message!!  : !m%s!!\n" FOS_TAB_4
                    "!ywill qos!!      : !m%s!!\n" FOS_TAB_4 "!ywill retain!!   : !m%s!!\n",
                    this->server_addr_.c_str(), client_id->toString().c_str(),
                    this->will_message_.get() ? this->will_message_->target.toString().c_str() : "<none>",
                    this->will_message_.get() ? this->will_message_->payload->toString().c_str() : "<none>",
                    this->will_message_.get() ? "1" : "<none>",
                    this->will_message_.get() ? FOS_BOOL_STR(this->will_message_->retain) : "<none>");
    }

  public:
    void stop() override {
      LOG_STRUCTURE(INFO, this, "Disconnecting from mqtt broker !g[!y%s!g]!!\n", this->server_addr_.c_str());
      native_mqtt_disconnect();
      Structure::stop();
    }

    void recv_publication(const Message_p &message) override {
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      this->write(id_p(message->target), message->payload, message->retain);
      LOG_PUBLISH(OK, *message);
    }

    void recv_subscription(const Subscription_p &subscription) override {
      check_availability("subscription");
      const bool mqtt_sub = !this->has_equal_subscription_pattern(furi_p(subscription->pattern));
      Structure::recv_subscription(subscription);
      if (mqtt_sub) {
        LOG_STRUCTURE(DEBUG, this, "Subscribing as no existing subscription found: %s\n",
                      subscription->toString().c_str());
        native_mqtt_subscribe(subscription);
      }
    }

    void recv_unsubscribe(const ID_p &source, const fURI_p &target) override {
      check_availability("unsubscribe");
      const bool mqtt_sub = this->has_equal_subscription_pattern(target);
      Structure::recv_unsubscribe(source, target);
      if (mqtt_sub && !this->has_equal_subscription_pattern(target)) {
        LOG_STRUCTURE(DEBUG, this, "Unsubscribing from mqtt broker as no existing subscription pattern found: %s\n",
                      target->toString().c_str());
        native_mqtt_unsubscribe(target);
      }
    }

    List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &furi) override {
      // FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const bool pattern_or_branch = furi->is_pattern() || furi->is_branch();
      const fURI temp = furi->is_branch() ? furi->extend("+") : *furi;
      auto thing = new std::atomic<List<Pair<ID_p, Obj_p>> *>(new List<Pair<ID_p, Obj_p>>());
      const auto source_id = ID(string("client_") + to_string(rand()));
      this->recv_subscription(
        subscription_p(source_id, temp, QoS::_1,
                       Insts::to_bcode([this, furi, thing](const Message_p &message) {
                         LOG_STRUCTURE(DEBUG, this, "subscription pattern %s matched: %s\n",
                                       furi->toString().c_str(), message->toString().c_str());
                         scheduler()->feed_local_watchdog();
                         thing->load()->push_back({id_p(message->target), message->payload});
                       })));
      ///////////////////////////////////////////////
      const time_t start_timestamp = time(nullptr);
      if (pattern_or_branch) {
        while (time(nullptr) - start_timestamp < 2) {
          this->native_mqtt_loop();
        }
      } else {
        while (thing->load()->empty()) {
          if (time(nullptr) - start_timestamp > 1)
            break;
          this->native_mqtt_loop();
        }
      }
      ///////////////////////////////////////////////
      this->recv_unsubscribe(id_p(source_id), furi_p(temp));
      const List<Pair<ID_p, Obj_p>> list = *thing->load();
      delete thing;
      return list;
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj) override {
      LOG_STRUCTURE(DEBUG, this, "writing to mqtt broker: %s\n", obj->toString().c_str());
      native_mqtt_publish(message_p(*id, obj, true));
    }

    void publish_retained(const Subscription_p &) override {
      // handled by mqtt broker
    }
  };
} // namespace fhatos
#endif
