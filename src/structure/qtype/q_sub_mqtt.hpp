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
#ifndef fhatos_q_sub_mqtt_hpp
#define fhatos_q_sub_mqtt_hpp

#include "../q_proc.hpp"
#include "../../fhatos.hpp"
#include "../pubsub.hpp"
#include "../../util/mutex_deque.hpp"
#include "../util/mqtt/mqtt_client.hpp"


namespace fhatos {
  class QSubMqtt final : public QProc {
  protected:
    ptr<MqttClient> mqtt;

  public:
    explicit QSubMqtt(const Rec_p &config, const ID_p &value_id = nullptr) : QProc(REC_FURI, value_id) {
      this->Obj::rec_set("pattern", vri("sub"));
      this->Obj::rec_set("config", config);
      this->mqtt = MqttClient::get_or_create(config->rec_get("broker")->uri_value(),
                                             config->rec_get("client")->uri_value());
    }

    static ptr<QSubMqtt> create(const Rec_p &config, const ID_p &value_id = nullptr) {
      //TYPE_SAVER("/fos/q/q_sub", Obj::to_rec());
      return make_shared<QSubMqtt>(config, value_id);
    }

    void loop() const override {
      this->mqtt->loop();
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      const fURI furi_no_query = furi.no_query();
      if(retain && POSITION::PRE == pos) {
        // unsubscribe
        this->mqtt->unsubscribe(*this->vid, furi);
        // if obj, subscribe
        if(!obj->is_noobj()) {
          if(obj->tid->equals("/fos/q/sub")) {
            this->mqtt->subscribe(make_shared<Subscription>(obj));
          } else {
            this->mqtt->subscribe(Subscription::create(this->vid, p_p(furi_no_query), obj));
          }
          LOG_WRITE(DEBUG, this,L("!m[!b{}!m]=!gsubscribe!m=>[!b{}!m]!!\n", "", /*subscription->source()->toString()*/
                                  furi_no_query.toString())          );
          // NEEDS ACCESS TO STRUCTURE?? this->publish_retained(subscription);
        }
        LOG_WRITE(TRACE, this,L("!ypre-wrote!! !b{}!! -> {}\n", furi_no_query.toString(), obj->toString()));
      }
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      /// pre-read
      Obj_p return_obj = Obj::to_noobj();
      const fURI furi_no_query = furi.no_query();
      Objs_p subs = Obj::to_objs();
      for(const Subscription_p &sub: *this->mqtt->subscriptions_) {
        if(furi_no_query.matches(*sub->pattern())) {
          subs->add_obj(sub);
        }
      }
      LOG_WRITE(TRACE, this,L("!ypre-read!! !b{}!! -> {}\n", furi_no_query.toString(), return_obj->toString()));
      return subs;
    }

    [[nodiscard]] ON_RESULT is_pre_read() const override {
      return ON_RESULT::ONLY_Q;
    }

    [[nodiscard]] ON_RESULT is_pre_write() const override {
      return ON_RESULT::ONLY_Q;
    }

    [[nodiscard]] ON_RESULT is_q_less_write() const override {
      return ON_RESULT::IGNORE_Q;
    }
  };
}
#endif
