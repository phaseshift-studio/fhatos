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

#include "util/obj_helper.hpp"

namespace fhatos {
#define RETAIN true
#define TRANSIENT false

#define LOG_SUBSCRIBE(rc, subscription)                                                                                \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gsubscribe!m=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!\n",            \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(),                                  \
      (subscription)->source().toString().c_str(), (subscription)->pattern().toString().c_str(),                           \
      (subscription)->on_recv()->toString().c_str())
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(), ((source)->toString().c_str()),   \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=>!m[!b%s!m]!!\n",                      \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(),                                  \
      ((message).payload()->toString().c_str()), (FOS_BOOL_STR((message).retain)),                                       \
      ((message).target().toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                                                         \
  LOG(((rc) == OK ? DEBUG : ERROR),                                                                                    \
      (((subscription).pattern().equals((message).target()))                                                               \
           ? "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern|target:!b%s!m]=!!%s!!\n"                                         \
           : "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern:%s][target:%s]=!!%s!!\n"),                                       \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((subscription).source().toString().c_str()), ((subscription).pattern().toString().c_str()),                         \
      ((subscription).pattern().equals((message).target())) ? ((message).payload()->toString().c_str())                      \
                                                        : ((message).target().toString().c_str()),                       \
      ((subscription).pattern().equals((message).target())) ? ((message).payload()->toString().c_str())                      \
                                                        : (message).payload()->toString.c_str())

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

  static const ID_p MESSAGE_FURI = id_p(FOS_SCHEME "/msg");

  struct Message final : Rec {
    explicit Message(const Rec_p &rec) :
      Rec(*rec) {
    }

    explicit Message(const ID &target, const Obj_p &payload, const bool retain) :
      Rec(rmap({
              {"target", vri(target)},
              {"payload", payload},
              {"retain", dool(retain)}}), OType::REC, MESSAGE_FURI) {
    }

    ID target() const {
      return ID(this->rec_get(vri("target"), [this]() {
        throw fError("message has no !ytarget!!: %s", this->toString().c_str());
      })->uri_value());
    }

    Obj_p payload() const {
      return this->rec_get(vri("payload"), [this]() {
        throw fError("message has no !ypayload!!: %s", this->toString().c_str());
      });
    }

    bool retain() const {
      return this->rec_get(vri("retain"), [this]() {
        throw fError("message has no !yretain!!: %s", this->toString().c_str());
      })->value<bool>();
    }

    static Message_p create(const ID &target, const Obj_p &payload, const bool retain) {
      return std::make_shared<Message>(target, payload, retain);
    }
  };

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

  static const ID_p SUBSCRIPTION_FURI = id_p(FOS_SCHEME "/sub");

  struct Subscription final : Rec {
    explicit Subscription(const Rec_p &rec) :
      Rec(*rec) {
    }

    explicit Subscription(const ID &source, const Pattern &pattern, const BCode_p &on_recv) :
      Rec(rmap({
              {"source", vri(source)},
              {"pattern", vri(pattern)},
              {":on_recv", on_recv}
          }), OType::REC, SUBSCRIPTION_FURI) {
    }

    ID source() const {
      return ID(this->rec_get("source", [this]() {
        throw fError("subscription has no !ysource!!: %s", this->toString().c_str());
      })->uri_value());
    }

    Pattern pattern() const {
      return Pattern(this->rec_get("pattern", [this]() {
        throw fError("subscription has no !ypattern!!: %s", this->toString().c_str());
      })->uri_value());
    }

    BCode_p on_recv() const {
      return this->rec_get(":on_recv", [this]() {
        throw fError("subscription has no !yon_recv!!: %s", this->toString().c_str());
      });
    }

    Obj_p process_message(const Message_p &message) const {
      return this->on_recv()->apply(message->payload());//, {message});
    }

    static Subscription_p create(const ID &source, const Pattern &pattern, const BCode_p &on_recv) {
      return make_shared<Subscription>(source, pattern, on_recv);
    }
  };
} // namespace fhatos

#endif