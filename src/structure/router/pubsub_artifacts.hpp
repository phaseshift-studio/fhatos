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


#ifndef fhatos_structure_subscription_hpp
#define fhatos_structure_subscription_hpp

#include <fhatos.hpp>

namespace fhatos {

  //////////////////////////////////////////////
  /////////////// MESSAGE STRUCT ///////////////
  //////////////////////////////////////////////
  struct Message {
  public:
    const ID source;
    const ID target;
    const Obj_p payload;
    const bool retain;

    [[nodiscard]] string toString() const {
      char temp[150];
      sprintf(temp, "!g[!b%s!g]!!=!y%s!![retain:%s]=>!g[!b%s!g]!!", source.toString().c_str(), payload->toString().c_str(),
              FOS_BOOL_STR(retain), target.toString().c_str());
      return {temp};
    }
  };

  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////
  enum class QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

  struct Subscription;
  using Subscription_p = ptr<Subscription>;
  using Message_p = ptr<Message>;
  struct Subscription {
    using Mail = Pair<const Subscription_p, const Message_p>;
    using Mail_p = ptr<Mail>;
    Mailbox<Mail_p> *mailbox = nullptr;
    fURI source;
    Pattern pattern;
    QoS qos = QoS::_1;
    Consumer<const Message_p> onRecv = [](const Message_p &) {};
    BCode_p onRecvBCode = nullptr;

    bool match(const ID &target) const { return this->pattern.matches(target); }

    void execute(const Message_p &message) const { onRecv(message); }

    [[nodiscard]] string toString() const {
      char temp[150];
      sprintf(temp, "[!b%s!m]=!gsubscribe!m[qos:%i]=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!", source.toString().c_str(),
              (uint8_t) qos, pattern.toString().c_str(), onRecvBCode ? onRecvBCode->toString().c_str() : "<c-impl>");
      return {temp};
    }
  };


  using Mail = Pair<const Subscription_p, const Message_p>;
  using Mail_p = ptr<Mail>;


} // namespace fhatos

#endif
