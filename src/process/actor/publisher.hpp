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
#ifndef fhatos_publisher_hpp
#define fhatos_publisher_hpp

#include <structure/router.hpp>
#include <structure/pubsub.hpp>

namespace fhatos {
  class Publisher {
  public:
    const ID_p __id;

    virtual ~Publisher() = default;

    explicit Publisher(const IDed *ided) : __id(ided->id()) {}

    explicit Publisher(const ID_p &id) : __id(id) {}


    /// SUBSCRIBE
    virtual RESPONSE_CODE subscribe(const Pattern &relativePattern, const Consumer<const ptr<Message> &> &onRecv,
                                    const QoS qos = QoS::_1) {
      return router()->route_subscription(share(Subscription{
              .source = *this->__id, .pattern = this->makeTopic(relativePattern), .qos = qos, .onRecv = onRecv}));
    }

    /// UNSUBSCRIBE
    virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {

      return router()->route_unsubscribe(this->__id, p_p(this->makeTopic(relativePattern)));
    }

    virtual RESPONSE_CODE unsubscribeSource() { return router()->route_unsubscribe(this->__id); }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /// PUBLISH
    RESPONSE_CODE publish(const ID &relativeTarget, const ptr<const Obj> &payload,
                          const bool retain = TRANSIENT_MESSAGE) const {
      return router()->route_message(share(Message{.source = *this->__id,
              .target = this->makeTopic(relativeTarget),
              .payload = PtrHelper::clone(*payload),
              .retain = retain}));
    }

    /////////////////////////////////////////////////////////////////////////////////////////

  private:
    fURI makeTopic(const fURI &relativeTopic) const {
      if (relativeTopic.empty())
        return relativeTopic;
      return relativeTopic.toString()[0] == '/' ? relativeTopic : this->__id->extend(relativeTopic.toString().c_str());
    }
  };
} // namespace fhatos

#endif
