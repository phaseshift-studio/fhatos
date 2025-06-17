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

#include "../../../fhatos.hpp"
#include "../../../structure/pubsub.hpp"
#include "../../../structure/q_proc.hpp"
#include "../../../util/mutex_deque.hpp"
#include "../sys/scheduler/thread/thread.hpp"
#define Q_SUB_TID FOS_URI "/q/sub"

namespace fhatos {
  class QSub final : public QProc {

  public:
    ptr<Post> post_;
    explicit QSub(const ID_p &value_id = nullptr) :
        QProc(id_p("/fos/q/sub"), value_id), post_(make_shared<LocalPost>()) {
      this->Obj::rec_set("pattern", vri("sub"));
    }

    static ptr<QSub> create(const ID &value_id) {
      // TYPE_SAVER("/fos/q/q_sub", Obj::to_rec());
      return make_shared<QSub>(value_id.empty() ? nullptr : id_p(value_id));
    }

    void loop() const override {
      /*while(!this->empty()) {
        FEED_WATCHDOG();
        Option<Mail> mail = this->next_mail();
        mail.value().first->apply(mail.value().second);
      }*/
      this->post_->loop();
    }


    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      const fURI furi_no_query = furi.no_query();
      if(retain && POSITION::PRE == pos) {
        // unsubscribe
        const ID source_id =
            Thread::current_thread().has_value() ? *Thread::current_thread().value()->thread_obj_->vid : *SCHEDULER_ID;
        if(obj->is_noobj()) {
          this->post_->unsubscribe(source_id, furi_no_query);
        } else {
          const Subscription_p sub =
              obj->tid->equals(SUBSCRIPTION_TID)
                  ? make_shared<Subscription>(obj)
                  : Subscription::create((Thread::current_thread().has_value()
                                              ? Thread::current_thread().value()->thread_obj_->vid
                                              : SCHEDULER_ID),
                                         p_p(furi_no_query), obj);
          this->post_->subscribe(sub);
        }
        LOG_WRITE(TRACE, this, L("!ypre-wrote!! !b{}!! -> {}\n", furi_no_query.toString(), obj->toString()));
      } else if(POSITION::Q_LESS == pos) {
        // publish
        this->post_->publish(Message::create(id_p(furi_no_query), obj, retain));
      }
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      /// pre-read
      const fURI furi_no_query = furi.no_query();
      const Objs_p subs = Obj::to_objs();
      for(const Subscription_p &sub: *this->post_->subscriptions_) {
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

    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          Q_SUB_TID, InstBuilder::build(Typer::singleton()->vid->add_component(Q_SUB_TID))
                         ->domain_range(NOOBJ_FURI, {0, 0}, REC_FURI, {1, 1})
                         ->inst_f([](const Obj_p &, const InstArgs &) {
                           return Obj::to_rec({{vri(MESSAGE_TID), Obj::to_rec({{"target", Obj::to_type(URI_FURI)},
                                                                               {"payload", Obj::to_bcode()},
                                                                               {"retain", Obj::to_type(BOOL_FURI)}})},
                                               {vri(SUBSCRIPTION_TID), Obj::to_rec({{"source", Obj::to_type(URI_FURI)},
                                                                                    {"pattern", Obj::to_type(URI_FURI)},
                                                                                    {"on_recv", Obj::to_bcode()}})},
                                               {vri(Q_SUB_TID), Obj::to_rec()}});
                         })
                         ->create());
    }
  };
} // namespace fhatos
#endif
