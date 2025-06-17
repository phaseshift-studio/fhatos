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
#include "../lang/mmadt/mmadt_obj.hpp"
#include "../lang/obj.hpp"
#include "../util/mutex_deque.hpp"
#include "../util/obj_helper.hpp"

#define SUBSCRIPTION_TID FOS_URI "/q/sub/sub"
#define MESSAGE_TID FOS_URI "/q/sub/msg"

namespace fhatos {
  using namespace mmadt;
#define RETAIN true
#define TRANSIENT false

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

  static auto ResponseCodes = Enums<RESPONSE_CODE>({{OK, "OK"},
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

  struct Message final : Rec {
    explicit Message(const Rec_p &rec) : Rec(*rec) {}

    explicit Message(const ID_p &target, const Obj_p &payload, const bool retain) :
        Rec(rmap({{"target", vri(target)}, {"payload", payload}, {"retain", dool(retain)}}), OType::REC,
            id_p(MESSAGE_TID)) {}

    [[nodiscard]] ID_p target() const { return id_p(this->rec_get("target")->uri_value()); }

    [[nodiscard]] Obj_p payload() const { return this->rec_get("payload"); }

    [[nodiscard]] bool retain() const { return this->rec_get("retain")->bool_value(); }

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
  using Mail = std::pair<const Subscription_p, const Message_p>;

  struct Subscription final : Rec {
    explicit Subscription(const Rec_p &rec) : Rec(*rec) {}

    void post() const { ROUTER_WRITE(this->pattern()->query("sub"), this->on_recv(), true); }

    explicit Subscription(const ID_p &source, const Pattern_p &pattern, const Obj_p &on_recv) :
        Rec(rmap({{"source", vri(source)}, {"pattern", vri(pattern)}, {"on_recv", on_recv}}), OType::REC,
            id_p(FOS_URI "/q/sub/sub")) {}

    [[nodiscard]] ID_p source() const { return id_p(this->rec_get("source")->uri_value()); }

    [[nodiscard]] Pattern_p pattern() const { return p_p(this->rec_get("pattern")->uri_value()); }

    [[nodiscard]] BCode_p on_recv() const { return this->rec_get("on_recv"); }

    static Subscription_p create(const ID_p &source, const Pattern_p &pattern, const Obj_p &on_recv) {
      return make_shared<Subscription>(source, pattern, on_recv->clone());
    }

    static Subscription_p create(const ID_p &source, const Pattern_p &pattern, const Cpp &on_recv) {
      return Subscription::create(
          source, pattern,
          InstBuilder::build(INST_FURI)
              ->inst_args(Obj::to_inst_args({{"target", __().from(__().block(vri("target")), noobj())},
                                             {"payload", __().from(__().block(vri("payload")), noobj())},
                                             {"retain", __().from(__().block(vri("retain")), noobj())}}))
              ->inst_f(on_recv)
              ->create());
    }

    void apply(const Message_p &message) const {
      const Obj_p payload = message->payload();
      ROUTER_EXEC_WITHIN_FRAME(
          "#",
          Obj::to_rec({{"target", vri(message->target())}, {"payload", payload}, {"retain", dool(message->retain())}}),
          [this, payload] { return mmADT::delift(this->on_recv())->apply(payload); });
    }
  };

  class Mailbox {
    ptr<MutexDeque<Mail>> mailbox_;
    // std::vector<Mail> mailbox_ = std::vector<Mail>();

  public:
    Mailbox() : mailbox_(std::move(std::make_shared<MutexDeque<Mail>>())) {}
    Mailbox(const Mailbox &other) : mailbox_(other.mailbox_) {}
    Mailbox(Mailbox &&other) noexcept : mailbox_(std::move(other.mailbox_)) { other.mailbox_ = nullptr; }
    virtual ~Mailbox() = default;
    void recv_mail(const Mail &&mail) const { this->mailbox_->push_back(mail); }
    bool empty() const { return this->mailbox_->empty(); }
    virtual void loop() {
      while(process_next_mail()) {
        FEED_WATCHDOG();
      }
    }
    bool process_next_mail() const {
      if(const auto o = this->mailbox_->pop_front(); o.has_value()) {
        o->first->apply(o->second);
        return true;
      }
      return false;
    }
    std::optional<Mail> next_mail() const { return this->mailbox_->pop_front(); }
  };

  class Post : public Mailbox {
  public:
    ptr<MutexDeque<Subscription_p>> subscriptions_;
    explicit Post() : Mailbox(), subscriptions_(make_shared<MutexDeque<Subscription_p>>()) {};
    Post(const Post &other) : Mailbox(other), subscriptions_(other.subscriptions_) {}
    Post(Post &&other) noexcept : Mailbox(other) {
      this->subscriptions_ = std::move(other.subscriptions_);
      other.subscriptions_ = nullptr;
    }
    ~Post() override = default;
    virtual void subscribe(const Subscription_p &subscription, bool async = true) = 0;
    virtual void unsubscribe(const ID &source, const Pattern &pattern, bool async = true) = 0;
    virtual void publish(const Message_p &message, bool async = true) const = 0;
    virtual void receive(const Message_p &message, bool async = true) const {
      for(const Subscription_p &sub: *this->subscriptions_) {
        if(message->target()->bimatches(*sub->pattern())) {
          const Obj_p source_obj = ROUTER_READ(*sub->source());
          const auto mail = Mail(sub, message);
          if(const auto source_mailbox = source_obj->get_model<Obj>(false);
             source_mailbox && source_mailbox->recv_payload(&mail)) {
            LOG_WRITE(DEBUG, source_obj.get(),
                      L("!yrouting mail!! !b{}!! - {} !g[!mmailbox!g]!!\n", message->toString(), sub->toString()));
          } else {
            LOG_WRITE(DEBUG, source_obj.get(),
                      L("!yreceiving mail!! !b{}!! -> {}\n", message->toString(), sub->toString()));
            this->recv_mail(std::move(mail));
          }
        }
      }
    }
  };

  class LocalPost final : public Post {
  public:
    explicit LocalPost() {};
    void publish(const Message_p &message, const bool async) const override { this->receive(message, async); }
    void subscribe(const Subscription_p &subscription, const bool async) override {
      this->subscriptions_->push_back(subscription);
      LOG_WRITE(DEBUG, Obj::to_noobj().get(),
                L("!m[!b{}!m]=!gsubscribe!m=>[!b{}!m]!!\n", "", /*subscription->source()->toString()*/
                  subscription->pattern()->toString()));
      // TODO: ROUTE TO NEW SUBSCRIPTION: const Obj_p result = ROUTER_READ(*sub->pattern()); }
    }

    void unsubscribe(const ID &source, const Pattern &pattern, const bool async) override {
      this->subscriptions_->remove_if([this, &pattern, &source](const Subscription_p &sub) {
        const bool removing = sub->source()->equals(source) && (sub->pattern()->matches(pattern));
        if(removing)
          LOG_WRITE(DEBUG, ROUTER_READ(source).get(),
                    L("!m[!b{}!m]=!gunsubscribe!m=>[!b{}!m]!!\n", sub->source()->toString(), pattern.toString()));
        return removing;
      });
    }
  };

} // namespace fhatos

#endif
