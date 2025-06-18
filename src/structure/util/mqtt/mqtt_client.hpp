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
#include "../../../lang/obj.hpp"
#include "../../../model/fos/sys/router/structure.hpp"

namespace fhatos {
  class MqttClient;
  static auto CLIENTS = Map<fURI, ptr<MqttClient>>();

  class MqttClient final : public Rec, public Post {
  public:
    std::any handler_;
    Runnable on_connect = [] {};
    uptr<MutexDeque<ID>> clients_ = make_unique<MutexDeque<ID>>();

    explicit MqttClient(const Rec_p &config);

    [[nodiscard]] ID broker() const { return this->rec_get("broker")->uri_value(); }

    [[nodiscard]] ID client() const { return this->rec_get("client")->uri_value(); }

    [[nodiscard]] bool is_connected() const;

    virtual void loop() override;

    [[nodiscard]] bool connect(const ID &source) const;

    [[nodiscard]] bool disconnect(const ID &source, bool async = true);

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
      auto mqtt = make_shared<MqttClient>(Obj::to_rec({{"broker", vri(broker)}, {"client", vri(client)}}));
      CLIENTS.insert_or_assign(broker, mqtt);
      return mqtt;
    }
    void subscribe(const Subscription_p &subscription, bool async) override;
    void unsubscribe(const ID &source, const Pattern &pattern, bool async) override;
    void publish(const Message_p &message, bool async) const override;
    // void receive(const Message_p &message, bool async = true) const override;
  };
} // namespace fhatos
#endif
