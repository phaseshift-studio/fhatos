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

#ifndef fhatos_meta_router_hpp
#define fhatos_meta_router_hpp

#include <fhatos.hpp>
//
#include <process/router/local_router.hpp>
#include <process/router/mqtt_router.hpp>
#include <process/router/router.hpp>
#include <process/router/message.hpp>
#include <structure/io/net/f_wifi.hpp>

namespace fhatos {
  template<typename LOCAL_ROUTER = LocalRouter<>,
    typename REMOTE_ROUTER = MqttRouter<> >
  class MetaRouter : public Router<> {
  protected:
    Router<> *select(const ID &target) {
      return false && this->id().isLocal(target)
               ? (Router<> *) LOCAL_ROUTER::singleton()
               : (Router<> *) REMOTE_ROUTER::singleton();
    }

  public:
    inline static MetaRouter *singleton() {
      static MetaRouter singleton = MetaRouter();
      LOCAL_ROUTER::singleton();
      REMOTE_ROUTER::singleton();
      return &singleton;
    }

    MetaRouter(const ID &id = Router::mintID("kernel", "router/meta")) : Router<>(id) {
    }

    ~MetaRouter() { this->clear(); }

    virtual RESPONSE_CODE clear() override {
      RESPONSE_CODE __rc1 = LOCAL_ROUTER::singleton()->clear();
      RESPONSE_CODE __rc2 = REMOTE_ROUTER::singleton()->clear();
      return __rc1 == RESPONSE_CODE::OK ? __rc2 : __rc1;
    }

    virtual const RESPONSE_CODE publish(const Message &message) override {
      return this->select(message.target)->publish(message);
    }

    virtual const RESPONSE_CODE
    subscribe(const Subscription &subscription) override {
      return this->select(subscription.pattern)->subscribe(subscription);
    }

    virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                            const Pattern &pattern) override {
      return this->select(pattern)->unsubscribe(source, pattern);
    }

    virtual const RESPONSE_CODE unsubscribeSource(const ID &source) override {
      const RESPONSE_CODE local =
          LOCAL_ROUTER::singleton()->unsubscribeSource(source);
      const RESPONSE_CODE remote =
          REMOTE_ROUTER::singleton()->unsubscribeSource(source);
      return local ? local : remote;
    }
  };
} // namespace fhatos

#endif
