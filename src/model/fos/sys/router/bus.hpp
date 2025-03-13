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
#ifndef fhatos_bus_hpp
#define fhatos_bus_hpp

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../structure/structure.hpp"
#include "../thread/fmutex.hpp"

namespace fhatos {
  const static ID BUS_FURI = ID("/sys/lib/bus");

  class Bus final : public Structure {
    fMutex map_mutex;

  public:
    explicit Bus(const Pattern &pattern, const ID_p &value_id = nullptr,
                 const Rec_p &config = Obj::to_rec()) : Structure(pattern, id_p(BUS_FURI), value_id, config) {
      // this->Obj::rec_set("config",config->rec_merge(Router::singleton()->rec_get("config/default_config")->clone()->rec_value()));
      LOG_WRITE(INFO, this, L("!ymapping !c{}!m==>!g{}!!\n",
                              config->get<fURI>("source").toString(),
                              config->get<fURI>("target").toString()));
    }

    static ptr<Bus> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                           const Rec_p &config = Obj::to_rec()) {
      return Structure::create<Bus>(pattern, value_id, config);
    }

    static void *import(const ID &import_id) {
      Router::import_structure<Bus>(import_id, BUS_FURI);
      return nullptr;
    }

    void stop() override {
      Structure::stop();
    }

  protected:
    /*void recv_unsubscribe(const ID &source, const fURI &target) override {
      //   Structure::recv_unsubscribe(source, target);
      const fURI from_furi = this->rec_get("config/source")->uri_value();
      const fURI to_furi = this->rec_get("config/target")->uri_value();
      const fURI new_furi = to_furi.extend(target.remove_subpath(from_furi.toString()));
      Router::singleton()->write(new_furi.query("sub"),Obj::to_noobj());
    }

    void recv_subscription(const Subscription_p &subscription) override {
      // Structure::recv_subscription(subscription);
      const fURI from_furi = this->rec_get("config/source")->uri_value();
      const fURI to_furi = this->rec_get("config/target")->uri_value();
      const fURI new_furi = to_furi.extend(subscription->pattern()->remove_subpath(from_furi.toString()));
      Router::singleton()->write(new_furi.query("sub"),
        Subscription::create(subscription->source(), p_p(new_furi), subscription->on_recv()));
    }*/

    void write(const fURI &furi, const Obj_p &obj, const bool retain = RETAIN) override {
      fURI from_furi = this->rec_get("config/source")->uri_value();
      fURI to_furi = this->rec_get("config/target")->uri_value();
      fURI new_furi = to_furi.extend(furi.remove_subpath(from_furi.toString()));
      ROUTER_WRITE(new_furi, obj, retain);
    }

    Obj_p read(const fURI &furi) override {
      fURI from_furi = this->rec_get("config/source")->uri_value();
      fURI to_furi = this->rec_get("config/target")->uri_value();
      fURI new_furi = to_furi.extend(furi.remove_subpath(from_furi.toString()));
      return ROUTER_READ(new_furi);
    }

    void write_raw_pairs(const ID &id, const Obj_p &obj, bool retain) override {
      throw fError("unreachable function");
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      throw fError("unreachable function");
    }
  };
} // namespace fhatos
#endif
