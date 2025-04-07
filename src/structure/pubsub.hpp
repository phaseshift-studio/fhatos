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

#ifndef fhatos_pubsub_hpp
#define fhatos_pubsub_hpp

#include "../fhatos.hpp"
#include  "../lang/obj.hpp"
#include "../util/obj_helper.hpp"
#include "../lang/mmadt/mmadt_obj.hpp"

namespace fhatos {
  using namespace mmadt;
#define RETAIN true
#define TRANSIENT false

#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? INFO : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                 \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(), ((source)->toString().c_str()),  \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=>!m[!b%s!m]!!\n",                      \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.to_chars(rc) + "!!").c_str(),                                  \
      ((message).payload()->toString().c_str()), (FOS_BOOL_STR((message).retain)),                                     \
      ((message).target().toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                                                         \
  LOG(((rc) == OK ? DEBUG : ERROR),                                                                                    \
      (((subscription).pattern().equals((message).target()))                                                           \
           ? "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern|target:!b%s!m]=!!%s!!\n"                                         \
           : "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern:%s][target:%s]=!!%s!!\n"),                                       \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((subscription).source().toString().c_str()), ((subscription).pattern().toString().c_str()),                     \
      ((subscription).pattern().equals((message).target())) ? ((message).payload()->toString().c_str())                \
                                                        : ((message).target().toString().c_str()),                     \
      ((subscription).pattern().equals((message).target())) ? ((message).payload()->toString().c_str())                \
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

  // static const ID_p MESSAGE_FURI = id_p(/*FOS_URI*/ "/fos/q/msg");
  // static const ID_p SUBSCRIPTION_FURI = id_p(/*FOS_URI*/ "/fos/q/sub");

  struct Message final : Rec {
    explicit Message(const Rec_p &rec) :
      Rec(*rec) {
    }

    explicit Message(const ID_p &target, const Obj_p &payload, const bool retain) :
      Rec(rmap({
              {"target", vri(target)},
              {"payload", payload},
              {"retain", dool(retain)}}), OType::REC, MESSAGE_FURI) {
    }

    ID_p target() const {
      return id_p(this->rec_get("target")->uri_value());
    }

    Obj_p payload() const {
      return this->rec_get("payload");
    }

    bool retain() const {
      return this->rec_get("retain")->bool_value();
    }

    static Message_p create(const ID_p &target, const Obj_p &payload, const bool retain) {
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


  struct Subscription final : Rec {
    explicit Subscription(const Rec_p &rec) :
      Rec(*rec) {
    }

    explicit Subscription(const ID_p &source, const Pattern_p &pattern, const Obj_p &on_recv) :
      Rec(rmap({
              {"source", vri(source)},
              {"pattern", vri(pattern)},
              {"on_recv", on_recv}
          }), OType::REC, SUBSCRIPTION_FURI) {
    }

    ID_p source() const {
      return id_p(this->rec_get("source")->uri_value());
    }

    Pattern_p pattern() const {
      return p_p(this->rec_get("pattern")->uri_value());
    }

    BCode_p on_recv() const {
      return this->rec_get("on_recv");
    }

    static Subscription_p create(const ID_p &source, const Pattern_p &pattern, const Obj_p &on_recv) {
      return make_shared<Subscription>(source, pattern, on_recv->clone());
    }

    static Subscription_p create(const ID_p &source, const Pattern_p &pattern, const Cpp &on_recv) {
      return Subscription::create(source, pattern, InstBuilder::build(INST_FURI)
                                  ->inst_args(Obj::to_inst_args({
                                      {"target", __().from("target", noobj())},
                                      {"payload", __().from("payload", noobj())},
                                      {"retain", __().from("retain", noobj())}}))
                                  ->inst_f(on_recv)->create());
    }

    void apply(const Message_p &message) const {
      const Uri_p target_obj = vri(message->target());
      const Obj_p payload_obj = message->payload();
      const Bool_p retain_obj = dool(message->retain());
      const bool is_no = payload_obj->is_noobj();
      const Inst_p pubsub_inst = InstBuilder::build("pubsub")->inst_args(to_rec({
              {"target", is_no && false ? target_obj : __().block(target_obj)},
              {"payload", is_no ? payload_obj : __().block(payload_obj)},
              {"retain", is_no ? retain_obj : __().block(retain_obj)}}))
          ->inst_f(this->on_recv())
          ->create();
      pubsub_inst->apply(payload_obj);
    }
  };
} // namespace fhatos

#endif
