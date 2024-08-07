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
#ifndef fhatos_meta_router_hpp
#define fhatos_meta_router_hpp

#include <fhatos.hpp>
//
#include <structure/router/local_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/router/router.hpp>

namespace fhatos {
  class MetaRouter : public Router {
  protected:
    Router *_local;
    Router *_global;
    Router *select(const ID &target) const { return (strlen(target.host()) == 0) ? this->_global : this->_local; }

  public:
    static MetaRouter *singleton(const ID &id = ID("/router/meta"), Router *local = LocalRouter::singleton(),
                                 Router *global = MqttRouter::singleton()) {
      static MetaRouter singleton = MetaRouter(id, local, global);
      return &singleton;
    }

    explicit MetaRouter(const ID &id = ID("/router/meta"), Router *local = LocalRouter::singleton(),
                        Router *global = MqttRouter::singleton()) : Router(id), _local(local), _global(global) {}

    RESPONSE_CODE clear(const bool subscriptions = true, const bool retains = true) override {
      const RESPONSE_CODE __rc1 = this->_local->clear(subscriptions, retains);
      const RESPONSE_CODE __rc2 = this->_global->clear(subscriptions, retains);
      return __rc1 == RESPONSE_CODE::OK ? __rc2 : __rc1;
    }

    RESPONSE_CODE publish(const Message &message) override { return this->select(message.target)->publish(message); }

    RESPONSE_CODE subscribe(const Subscription &subscription) override {
      return this->select(subscription.pattern)->subscribe(subscription);
    }

    RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return this->select(pattern)->unsubscribe(source, pattern);
    }

    RESPONSE_CODE unsubscribeSource(const ID &source) override {
      return this->select(source)->unsubscribeSource(source);
    }
  };
} // namespace fhatos

#endif
