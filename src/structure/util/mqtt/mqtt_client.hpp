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
#ifndef fhatos_mqtt_client_hpp
#define fhatos_mqtt_client_hpp

#include "../../../fhatos.hpp"
#include "../../structure.hpp"
#include "../../../lang/obj.hpp"
#include "../../../model/fos/util/log.hpp"

namespace fhatos {
  class MqttClient;
  static auto CLIENTS = Map<fURI, ptr<MqttClient>>();

  class MqttClient final : Rec {
  public:
    std::any handler_;
    uptr<MutexDeque<ID>> clients_ = make_unique<MutexDeque<ID>>();
    uptr<MutexDeque<Subscription_p>> subscriptions_ = make_unique<MutexDeque<Subscription_p>>();

    explicit MqttClient(const Rec_p &config);

    [[nodiscard]] ID broker() const {
      return this->rec_get("broker")->uri_value();
    }

    [[nodiscard]] ID client() const {
      return this->rec_get("client")->uri_value();
    }

    [[nodiscard]] bool is_connected() const;

    /* virtual void recv_unsubscribe(const ID &source, const fURI &target) {
          if(!this->available_.load())
            LOG_WRITE(ERROR, this, L("!yunable to unsubscribe!! {} from {}\n", source.toString(), target.toString()));
          else {
            this->subscriptions_->remove_if(
                [this,source, target](const Subscription_p &sub) {
                  const bool removing = sub->source()->equals(source) && (sub->pattern()->matches(target));
                  if(removing)
                    LOG_WRITE(DEBUG, this,
                              L("!m[!b{}!m]=!gunsubscribe!m=>[!b{}!m]!!\n", source.toString(), target.toString()));
                  return removing;
                });
            this->rec_set("sub", lst(LstList(this->subscriptions_->begin(), this->subscriptions_->end())));
            this->save();
          }
        }

        virtual void recv_subscription(const Subscription_p &subscription) {
          if(!this->available_.load()) {
            LOG_WRITE(ERROR, this, L("!yunable to receive!! {}\n", subscription->toString()));
            return;
          }
          LOG_WRITE(DEBUG, this, L("!yreceived!! {}\n", subscription->toString()));
          /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
          this->recv_unsubscribe(*subscription->source(), *subscription->pattern());
          if(!subscription->on_recv()->is_noobj()) {
            /////////////// ADD NEW SUBSCRIPTION
            this->rec_get("sub")->lst_add(subscription);
            this->save();
            this->subscriptions_->push_back(subscription);
            LOG_WRITE(DEBUG, this,L("!m[!b{}!m]=!gsubscribe!m=>[!b{}!m]!!\n", subscription->source()->toString(),
                                    pattern->toString())            );
            /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
            this->publish_retained(subscription);
          }
        }*/

    void subscribe(const Subscription_p &subscription) const;

    void unsubscribe(const ID &source, const fURI &pattern) const;

    void publish(const Message_p &message) const;

    void receive(const Message_p &message) const {
      LOG_WRITE(DEBUG, this, L("{} received\n", message->toString()));
      this->subscriptions_->forEach([this,&message](const Subscription_p &sub) {
        if(this->clients_->find([this,sub](const ID &client) {
          return sub->source()->equals(client);
        }).has_value()) {
          if(message->target()->matches(*sub->pattern())) {
            if(sub->on_recv().get())
              sub->apply(message);
          }
        }
      });
    }

    bool connect(const ID &source) const;

    bool disconnect(const ID &source) const;

    void loop();

    static BObj_p make_bobj(const Obj_p &payload, const bool retain) {
      const Lst_p lst = Obj::to_lst({payload, dool(retain)});
      return lst->serialize();
    }

    static Pair<Obj_p, bool> make_payload(const BObj_p &bobj) {
      const Lst_p lst = Obj::deserialize(bobj);
      return {lst->lst_value()->at(0), lst->lst_value()->at(1)->bool_value()};
    }


    static ptr<MqttClient> get_or_create(const fURI &broker, const fURI &client) {
      if(CLIENTS.count(broker))
        return CLIENTS.at(broker);
      ptr<MqttClient> mqtt = make_shared<MqttClient>(
          Obj::to_rec({{"broker", vri(broker)}, {"client", vri(client)}}));
      CLIENTS.insert_or_assign(broker, mqtt);
      return mqtt;
    }
  };

} // namespace fhatos
#endif
