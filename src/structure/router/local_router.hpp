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
#ifndef fhatos_local_router_hpp
#define fhatos_local_router_hpp

#include <fhatos.hpp>
//
#include <structure/router/rooter.hpp>
#include <structure/router/router.hpp>
#include <util/pubsub_manager.hpp>

namespace fhatos {
  class LocalRouter final : public Router {
  protected:
    PubSubManager _manager;

    explicit LocalRouter(const ID &id = ID("/router/local/")) :
        Router(id, ROUTER_LEVEL::LOCAL_ROUTER), _manager(PubSubManager(true)) {}


  public:
    static LocalRouter *singleton(const ID &id = ID("/router/local/")) {
      static LocalRouter local = LocalRouter(id);
      return &local;
    }

    uint retainSize() const override { return this->_manager.retainSize(); }

    uint subscriptionSize() const { return this->_manager.subscriptionSize(); }

    RESPONSE_CODE clear(const bool subscriptions, const bool retains) override {
      return this->_manager.clear(subscriptions, retains);
    }

    RESPONSE_CODE publish(const Message &message) override { return this->_manager.publish(message); }

    RESPONSE_CODE subscribe(const Subscription &subscription) override {
      return this->_manager.subscribe(subscription);
    }

    RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return this->_manager.unsubscribe(source, pattern);
    }

    RESPONSE_CODE unsubscribeSource(const ID &source) override { return this->_manager.unsubscribeSource(source); }

    const string toString() const override { return "LocalRouter"; }
  };
} // namespace fhatos
#endif
