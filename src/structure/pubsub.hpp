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


#ifndef fhatos_structure_subscription_hpp
#define fhatos_structure_subscription_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {
#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

#define LOG_SUBSCRIBE(rc, subscription)                                                                                \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gsubscribe!m=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!\n",            \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(),                                  \
      (subscription)->source.toString().c_str(),                                                                       \
      (subscription)->pattern.toString().c_str(), (subscription)->on_recv->toString().c_str())
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(), ((source).toString().c_str()),   \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=>!m[!b%s!m]!!\n",                      \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(),                                  \
      ((message).payload->toString().c_str()), (FOS_BOOL_STR((message).retain)),                                       \
      ((message).target.toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                                                         \
  LOG(((rc) == OK ? DEBUG : ERROR),                                                                                    \
      (((subscription).pattern.equals((message).target))                                                               \
           ? "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern|target:!b%s!m]=!!%s!!\n"                                         \
           : "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern:%s][target:%s]=!!%s!!\n"),                                       \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((subscription).source.toString().c_str()), ((subscription).pattern.toString().c_str()),                         \
      ((subscription).pattern.equals((message).target)) ? ((message).payload->toString().c_str())                      \
                                                        : ((message).target.toString().c_str()),                       \
      ((subscription).pattern.equals((message).target)) ? ((message).payload->toString().c_str())                      \
                                                        : ((message).payload->toString)().c_str())

  //////////////////////////////////////////////
  /////////////// ERROR MESSAGES ///////////////
  //////////////////////////////////////////////

  enum RESPONSE_CODE {
    OK = 0,
    NO_TARGETS,
    REPEAT_SUBSCRIPTION,
    NO_SUBSCRIPTION,
    NO_MESSAGE,
    ROUTER_ERROR,
    MUTEX_TIMEOUT,
    MUTEX_LOCKOUT
  };

  static Enums<RESPONSE_CODE> ResponseCodes = Enums<RESPONSE_CODE>({{OK, "OK"},
    {NO_TARGETS, "no targets"},
    {REPEAT_SUBSCRIPTION, "repeat subscription"},
    {NO_SUBSCRIPTION, "no subscription"},
    {NO_MESSAGE, "no message"},
    {ROUTER_ERROR, "internal router error"},
    {MUTEX_TIMEOUT, "router timeout"}});

  //////////////////////////////////////////////
  /////////////// MESSAGE STRUCT ///////////////
  //////////////////////////////////////////////
  struct Message;
  using Message_p = ptr<Message>;

  struct Message {
    Message(const ID &target,
            const Obj_p &payload,
            const bool retain) : target(ID(target)),
                                 payload(payload),
                                 retain(retain) {
    }
    const ID target;
    const Obj_p payload;
    const bool retain;

    [[nodiscard]] string toString() const {
      char temp[1024];
      snprintf(temp, 1024, "!g[!b%s!g]!!=[retain:%s]=>!g[!b%s!g]!!", this->payload->toString().c_str(),
               FOS_BOOL_STR(this->retain), this->target.toString().c_str());
      return {temp};
    }

    [[nodiscard]] Rec_p to_rec() const {
      return Obj::to_rec({{vri(":target"), vri(this->target)},
                           {vri(":payload"), this->payload->clone()},
                           {vri(":retain"), dool(this->retain)}},
                         id_p(REC_FURI->extend("msg")));
    }
  };

  inline Message_p message_p(const ID &target, const Obj_p &payload, const bool retain) {
    return std::make_shared<Message>(target, payload, retain);
  }

  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////

  struct Subscription;
  using Subscription_p = ptr<Subscription>;
  using Message_p = ptr<Message>;
  using Mail = Pair<const Subscription_p, const Message_p>;
  using Mail_p = ptr<Mail>;

  inline Mail_p mail_p(const Subscription_p &subscription, const Message_p &message) {
    return make_shared<Mail>(subscription, message);
  }

  struct Mailbox {
    virtual ~Mailbox() = default;

    virtual bool recv_mail(const Mail_p &mail) = 0;
  };

  struct Subscription {
    Subscription(const ID &source,
                 const Pattern &pattern,
                 const BCode_p &on_recv): source(ID(source)),
                                          pattern(Pattern(pattern)),
                                          on_recv(on_recv) {
    }

    const ID source;
    const Pattern pattern;
    const BCode_p on_recv;

    [[nodiscard]] Rec_p to_rec() const {
      return rec({{vri(":source"), vri(source)},
                   {vri(":pattern"), vri(pattern)},
                   {vri(":on_recv"), on_recv}},
                 id_p(REC_FURI->extend("sub")));
    }

    [[nodiscard]] string toString() const {
      char temp[1024];
      snprintf(temp, 1024, "[!b%s!m]=!gsubscribe!m=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!",
               source.toString().c_str()/*, static_cast<uint8_t>(qos)*/, pattern.toString().c_str(),
               on_recv->toString().c_str());
      return {temp};
    }
  };

  inline Subscription_p subscription_p(const ID &source, const Pattern &pattern, const BCode_p &on_recv) {
    return make_shared<Subscription>(source, pattern, on_recv);
  }
} // namespace fhatos

#endif
