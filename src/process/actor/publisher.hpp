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
  class Publisher final {
  public:
    const ID_p __id;

    virtual ~Publisher() = default;

    explicit Publisher(const IDed *ided) : __id(ided->id()) {
    }

    explicit Publisher(const ID_p &id) : __id(id) {
    }


    /// SUBSCRIBE
    virtual void subscribe(const Pattern &relative_pattern, const Consumer<const ptr<Message> &> &on_recv,
                           const QoS qos = QoS::_1) {
      router()->route_subscription(share(Subscription{
        .source = fURI(*this->__id), .pattern = this->make_topic(relative_pattern), .qos = qos, .onRecv = on_recv}));
    }

    /// UNSUBSCRIBE
    virtual void unsubscribe(const Pattern &relative_pattern) {
      router()->route_unsubscribe(this->__id, p_p(this->make_topic(relative_pattern)));
    }

    virtual void unsubscribe_source() { return router()->route_unsubscribe(this->__id); }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /// PUBLISH
    void publish(const ID &relative_target, const ptr<const Obj> &payload,
                 const bool retain = TRANSIENT_MESSAGE) const {
      router()->route_message(share(Message{.source = *this->__id,
        .target = this->make_topic(relative_target),
        .payload = PtrHelper::clone(*payload),
        .retain = retain}));
    }

    /////////////////////////////////////////////////////////////////////////////////////////

  private:
    fURI make_topic(const fURI &relative_topic) const {
      if (relative_topic.empty())
        return relative_topic;
      return relative_topic.toString()[0] == '/' ? relative_topic : this->__id->extend(relative_topic.toString().c_str());
    }
  };
} // namespace fhatos

#endif
