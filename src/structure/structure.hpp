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
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/pubsub.hpp>
#include <util/enums.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>

namespace fhatos {
  enum class SType {
    READ, WRITE, READWRITE
  };
  static const Enums<SType> StructureTypes =
          Enums<SType>({{SType::READ,      "read"},
                        {SType::WRITE,     "write"},
                        {SType::READWRITE, "readwrite"}});

  class Structure : public Patterned {

  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<List<Subscription_p>> subscriptions = std::make_shared<List<Subscription_p>>();
    MutexRW<> mutex = MutexRW<>();
    std::atomic_bool available_ = std::atomic_bool(false);
    bool remote_retains = false;

  public:
    const SType stype;

    explicit Structure(const Pattern &pattern, const SType stype) : Patterned(p_p(pattern)), stype(stype) {}

    bool available() const { return this->available_.load(); }

    virtual void setup() {
      if (this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
      this->available_.store(true);
    }

    virtual void loop() {
      if (!this->available_.load())
        throw fError(FURI_WRAP " !ystructure!! is closed\n", this->pattern()->toString().c_str());
      Option<Mail_p> mail;
      while ((mail = this->outbox_->pop_back()).has_value()) {
        LOG_STRUCTURE(TRACE, this, "Processing message %s for subscription %s\n",
                      mail.value()->second->toString().c_str(), mail.value()->first->toString().c_str());
        mail.value()->first->onRecv(mail.value()->second);
      }
    }

    virtual void stop() {
      if (!this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already closed");
      this->available_.store(false);
      this->subscriptions->clear();
      this->outbox_->clear(false);
    }

    virtual void recv_unsubscribe(const ID_p &source, const fURI_p &target) {
      if (!this->available_.load())
        return;
      this->subscriptions->erase(remove_if(this->subscriptions->begin(), this->subscriptions->end(),
                                           [source, target](const Subscription_p &sub) {
                                             LOG_UNSUBSCRIBE(OK, *source, target);
                                             return sub->source.equals(*source) && sub->pattern.matches(*target);
                                           }),
                                 this->subscriptions->end());
    }

    virtual void recv_subscription(const Subscription_p &subscription) {
      if (!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to receive!! %s\n", subscription->toString().c_str());
        return;
      }
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", subscription->toString().c_str());
      this->mutex.write<void *>([this, subscription]() {
        /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
        this->subscriptions->erase(remove_if(this->subscriptions->begin(), this->subscriptions->end(),
                                             [subscription](const Subscription_p &sub) {
                                               return sub->source.equals(subscription->source) &&
                                                      sub->pattern.matches(subscription->pattern);
                                             }),
                                   this->subscriptions->end());
        /////////////// ADD NEW SUBSCRIPTION
        if (subscription->onRecv) { // not an unsubscribe event // todo: is it even possible to be in this state?
          const ptr<Subscription> sub_ptr = share<Subscription>(*subscription);
          this->subscriptions->push_back(sub_ptr);
          if (!this->remote_retains) {
            const Obj_p objx = this->read(p_p(subscription->pattern), id_p(subscription->source)); // get any retains
            if (objx->is_objs()) {
              for (const auto &obj: *objx->objs_value()) {
                this->outbox_->push_back(share(Mail(
                        {subscription,
                         share(Message{.source = ID("anon_src"),
                                 .target = ID("anon_tgt"),
                                 .payload = obj,
                                 .retain = true})}))); // TODO: need both source of the retain and the target of obj
              }
            } /* else {
           if (!objx->is_noobj()) {
             this->outbox_->push_back(share(
                 Mail({subscription,
                       share(Message{.source = ID("anon_src"),
                                     .target = ID("anon_tgt"),
                                     .payload = objx,
                                     .retain = true})}))); // TODO: need both source of the retain and the target of obj
           }*/
          }
          LOG_SUBSCRIBE(OK, subscription);
        } else {
          LOG_UNSUBSCRIBE(OK, subscription->source, &subscription->pattern);
        }
        return nullptr;
      });
    }

    virtual void recv_message(const Message_p &message) {
      if (!this->available_.load())
        return;
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      if (message->retain) {
        this->write(id_p(message->target), message->payload, id_p(message->source));
      }
      auto rc = mutex.read<RESPONSE_CODE>([this, message]() {
        RESPONSE_CODE rc2 = NO_SUBSCRIPTION;
        for (const auto &subscription: *this->subscriptions) {
          if (message->target.matches(subscription->pattern)) {
            rc2 = OK;
            Subscription_p sub = share(Subscription(*subscription));
            this->outbox_->push_back(share(Mail{sub, message}));
            LOG(DEBUG, "%s !yrouted to!! %s\n", message->toString().c_str(), subscription->toString().c_str());
          }
        }
        return rc2;
      });
      MESSAGE_INTERCEPT(message->source, message->target, message->payload, message->retain);
      LOG_PUBLISH(rc, *message);
    }

    virtual void remove(const ID_p &id, const ID_p &source) { this->write(id, Obj::to_noobj(), source); }

    virtual Obj_p read(const fURI_p &furi, const ID_p &source) = 0;

    virtual Obj_p read(const fURI_p &furi) { return this->read(furi, id_p(FOS_DEFAULT_SOURCE_ID)); }

    virtual void write(const ID_p &id, const Obj_p &obj, const ID_p &source) = 0;

    virtual void write(const ID_p &id, const Obj_p &obj) { this->write(id, obj, id_p(FOS_DEFAULT_SOURCE_ID)); }
  };

  using Structure_p = ptr<Structure>;

} // namespace fhatos

#endif
