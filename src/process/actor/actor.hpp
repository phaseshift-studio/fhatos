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
#ifndef fhatos_actor_hpp
#define fhatos_actor_hpp

#include <fhatos.hpp>
//
#include <process/actor/mailbox.hpp>
#include <process/actor/publisher.hpp>
#include <structure/router/local_router.hpp>
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/router/router.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/structure.hpp>
#include <structure/stype/empty.hpp>
#include "structure/stype/space.hpp"

#include <language/exts.hpp>

namespace fhatos {
  template<typename PROCESS = Coroutine, typename STRUCTURE = Empty>
  class Actor : public PROCESS, public STRUCTURE {

  protected:
    MutexDeque<Mail_p> *inbox = new MutexDeque<Mail_p>();
    MutexDeque<Mail_p> *outbox = new MutexDeque<Mail_p>();
    List<Subscription_p> *subscriptions = new List<Subscription_p>();
    MutexRW<> mutex = MutexRW<>();

  public:
    explicit Actor(const ID &id, const Pattern &pattern) :
        PROCESS(id), STRUCTURE(pattern), Publisher(this, this), Mailbox<ptr<Mail>>() {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      static_assert(std::is_base_of_v<Structure, STRUCTURE>);
    }

    ~Actor() override { this->inbox->clear(); }

    // PAYLOAD BOX METHODS
    bool push(const ptr<Mail> mail) override { return this->running() && this->inbox->push_back(mail); }

    uint16_t size() override { return inbox->size(); }

    virtual void send_subscription(const Subscription_p &outgoing) {
      Rooter::singleton()->route_subscription(outgoing);
    }
    void recv_subscription(const Subscription_p &incoming) {
      this->mutex.template write<RESPONSE_CODE>([this, incoming]() {
        RESPONSE_CODE _rc = OK;
        /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
        this->subscriptions->erase(remove_if(this->subscriptions->begin(), this->subscriptions->end(),
                                             [incoming](const Subscription_p &sub) {
                                               return sub->source.equals(incoming->source) &&
                                                      sub->pattern.matches(incoming->pattern);
                                             }),
                                   this->subscriptions->end());
        /////////////// ADD NEW SUBSCRIPTION
        if (incoming->onRecv) { // not an unsubscribe event
          this->subscriptions->push_back(incoming);
          const List<Pair<fURI_p, Obj_p>> payload = this->read(incoming->pattern, incoming->source);
          for (const auto &[furi, obj]: payload) {
            const Mail_p mail = share(Mail(
                {incoming, share(Message{.target = *furi, .source = *this->id(), .payload = obj, .retain = true})}));
            if (incoming->mailbox) {
              incoming->mailbox->push(mail);
            } else {
              this->outgoing->push_back(mail);
            }
          }
          LOG_SUBSCRIBE(OK, incoming);
        } else {
          LOG_UNSUBSCRIBE(OK, incoming->source, &incoming->pattern);
        }
      });
    }

    virtual void send_message(const Message_p &outgoing) { Rooter::singleton()->route_message(outgoing); }


    void recv_message(const Message_p &incoming) {
      if (incoming->retain) {
        this->write(incoming->target, incoming->payload, incoming->source);
      }
      mutex.read<void *>([this, incoming]() {
        for (const auto &subscription: *this->subscriptions) {
          if (subscription->pattern.matches(incoming->target)) {
            const Mail_p mail = share(Mail{subscription, incoming});
            if (subscription->mailbox) {
              subscription->mailbox->push(mail);
            } else {
              this->outbox->push_back(mail);
            }
          }
        }
        return nullptr;
      });
    }
    bool active() { return this->available() && this->running(); }

    /// PROCESS METHODS
    //////////////////////////////////////////////////// SETUP
    void setup() override {
      PROCESS::setup();
      STRUCTURE::setup();
    }
    //////////////////////////////////////////////////// STOP
    void stop() override {
      PROCESS::stop();
      STRUCTURE::stop();
      if (const RESPONSE_CODE _rc = this->unsubscribeSource()) {
        LOG(ERROR, "Actor %s stop error: %s\n", this->id()->toString().c_str(), RESPONSE_CODE_STR(_rc));
      }
      this->inbox.clear();
    }
    //////////////////////////////////////////////////// LOOP
    void loop() override {
      PROCESS::loop();
      STRUCTURE::loop();
      const Option<ptr<Mail>> mail = this->inbox.pop_back();
      if (mail.has_value())
        mail->get()->first->execute(mail->get()->second);
    }
  };
} // namespace fhatos
#endif
