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
#ifndef fhatos_q_sub_hpp
#define fhatos_q_sub_hpp

#include "../../fhatos.hpp"
#include "../../model/fos/sys/scheduler/thread/thread.hpp"
#include "../../util/mutex_deque.hpp"
#include "../pubsub.hpp"
#include "../q_proc.hpp"

namespace fhatos {
  class QSub final : public QProc {
  protected:
    ptr<MutexDeque<Mail>> outbox_ = std::make_shared<MutexDeque<Mail>>();
    ptr<MutexDeque<Subscription_p>> subscriptions_ = std::make_shared<MutexDeque<Subscription_p>>();

  public:
    explicit QSub(const ID_p &value_id = nullptr) : QProc(id_p("/fos/q/sub"), value_id) {
      this->Obj::rec_set("pattern", vri("sub"));
    }

    static ptr<QSub> create(const ID &value_id) {
      // TYPE_SAVER("/fos/q/q_sub", Obj::to_rec());
      return make_shared<QSub>(value_id.empty() ? nullptr : id_p(value_id));
    }

    void loop() const override {
      while(!this->outbox_->empty()) {
        FEED_WATCHDOG();
        Option<Mail> mail = this->outbox_->pop_front();
        LOG_WRITE(
            TRACE, this,
            L("!yprocessing mail!! !b{}!! -> {}\n", mail.value().second->toString(), mail.value().first->toString()));
        mail.value().first->apply(mail.value().second);
      }
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      const fURI furi_no_query = furi.no_query();
      if(retain && POSITION::PRE == pos) {
        // unsubscribe
        const ID source_id =
            Thread::current_thread().has_value() ? *Thread::current_thread().value()->thread_obj_->vid : *SCHEDULER_ID;
        this->subscriptions_->remove_if([this, &furi_no_query, &source_id](const Subscription_p &sub) {
          const bool removing = sub->source()->equals(source_id) && (sub->pattern()->matches(furi_no_query));
          if(removing)
            LOG_WRITE(
                DEBUG, this,
                L("!m[!b{}!m]=!gunsubscribe!m=>[!b{}!m]!!\n", sub->source()->toString(), furi_no_query.toString()));
          return removing;
        });
        // if obj, subscribe
        if(!obj->is_noobj()) {
          if(obj->tid->equals("/fos/q/sub")) {
            this->subscriptions_->push_back(make_shared<Subscription>(obj));
          } else {
            this->subscriptions_->push_back(Subscription::create(
                (Thread::current_thread().has_value() ? Thread::current_thread().value()->thread_obj_->vid
                                                      : SCHEDULER_ID),
                p_p(furi_no_query), obj));
          }
          LOG_WRITE(DEBUG, this,
                    L("!m[!b{}!m]=!gsubscribe!m=>[!b{}!m]!!\n", "", /*subscription->source()->toString()*/
                      furi_no_query.toString()));
          // NEEDS ACCESS TO STRUCTURE?? this->publish_retained(subscription);
        }
        LOG_WRITE(TRACE, this, L("!ypre-wrote!! !b{}!! -> {}\n", furi_no_query.toString(), obj->toString()));
      } else if(POSITION::Q_LESS == pos) {
        // publish
        for(const Subscription_p &sub: *this->subscriptions_) {
          if(furi_no_query.bimatches(*sub->pattern())) {
            const Message_p msg = Message::create(id_p(furi_no_query), obj, retain);
            LOG_WRITE(DEBUG, this, L("!ysending mail!! !b{}!! -> {}\n", msg->toString(), sub->toString()));
            this->outbox_->push_back(Mail(sub, msg));
          }
        }
      }
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      /// pre-read
      const fURI furi_no_query = furi.no_query();
      const Objs_p subs = Obj::to_objs();
      for(const Subscription_p &sub: *this->subscriptions_) {
        if(furi_no_query.matches(*sub->pattern())) {
          subs->add_obj(sub);
        }
      }
      LOG_WRITE(TRACE, this, L("!ypre-read!! !b{}!! -> {}\n", furi_no_query.toString(), subs->toString()));
      return subs;
    }

    [[nodiscard]] ON_RESULT is_pre_read() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_pre_write() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_q_less_write() const override { return ON_RESULT::IGNORE_Q; }

    ///////////////////////////////////////////////////////////////////

    /*virtual void distribute_to_subscribers(const Message_p &message) {
      this->subscriptions_->forEach([this,message](const Subscription_p &subscription) {
        if(message->target()->matches(*subscription->pattern()))
          this->outbox_->push_back(mail_p(subscription, message));
      });
    }

    bool has_equal_subscription_pattern(const fURI &topic, const ID &source = "") const {
      return this->subscriptions_->exists([source,topic](const Subscription_p &sub) {
        if(source.empty() && !source.equals(*sub->source()))
          return false;
        if(topic.equals(*sub->pattern()))
          return true;
        return false;
      });
    }*/
  };
} // namespace fhatos
#endif
