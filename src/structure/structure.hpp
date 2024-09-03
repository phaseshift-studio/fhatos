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
      Enums<SType>({
        {SType::READ, "read"},
        {SType::WRITE, "write"},
        {SType::READWRITE, "readwrite"}
      });

  class Structure : public Patterned {
  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<List<Subscription_p>> subscriptions = std::make_shared<List<Subscription_p>>();
    MutexRW<> mutex = MutexRW<>();
    std::atomic_bool available_ = std::atomic_bool(false);
    bool remote_retains = false;

  public:
    const SType stype;

    explicit Structure(const Pattern &pattern, const SType stype): Patterned(p_p(pattern)), stype(stype) {
    }

    [[nodiscard]] bool available() const {
      return this->available_.load();
    }

    virtual void setup() {
      if (this->available_.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
        return;
      }
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
      this->subscriptions->clear();
      this->outbox_->clear(false);
      this->available_.store(false);
    }

    /////////////////////////////////////////////////

    virtual void recv_unsubscribe(const ID_p &source, const fURI_p &target) {
      if (!this->available_.load())
        return;
      this->subscriptions->erase(remove_if(this->subscriptions->begin(), this->subscriptions->end(),
                                           [source, target](const Subscription_p &sub) {
                                             const bool removing = sub->source.equals(*source) && (
                                                                     target->matches(sub->pattern) || sub->pattern.
                                                                     matches(*target));
                                             if (removing)
                                               LOG_UNSUBSCRIBE(OK, *source, target);
                                             return removing;
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
        this->subscriptions->push_back(subscription);
        /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
        if (!this->remote_retains) {
          this->write_retained(subscription);
          LOG_SUBSCRIBE(OK, subscription);
        }
        return nullptr;
      });
    }

    virtual RESPONSE_CODE recv_message(const Message_p &message) {
      if (!this->available_.load())
        return ROUTER_ERROR;
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      this->write(id_p(message->target), message->payload, id_p(message->source), message->retain);
      MESSAGE_INTERCEPT(message->source, message->target, message->payload, message->retain);
      LOG_PUBLISH(OK, *message);
      return OK;
    }

    virtual void remove(const ID_p &id, const ID_p &source) {
      this->write(id, Obj::to_noobj(), source, RETAIN_MESSAGE);
    }

    virtual void write_retained(const Subscription_p &subscription) = 0;

    virtual Obj_p read(const fURI_p &furi, const ID_p &source) = 0;

    virtual Obj_p read(const fURI_p &furi) { return this->read(furi, id_p(FOS_DEFAULT_SOURCE_ID)); }

    virtual void write(const ID_p &id, const Obj_p &obj, const ID_p &source, bool retain) = 0;

    virtual void write(const ID_p &id, const Obj_p &obj, bool retain) {
      this->write(id, obj, id_p(FOS_DEFAULT_SOURCE_ID), retain);
    }

  protected:
    void distribute_to_subscriptions(const Message_p &message) {
      for (const Subscription_p &sub: *this->subscriptions) {
        if (message->target.matches(sub->pattern))
          this->outbox_->push_back(share(Mail(sub, message)));
      }
    }

    Lst_p get_subscription_lst() {
      List<Obj_p> list;
      for (const Subscription_p &sub: *this->subscriptions) {
        const Rec_p sub_rec = Obj::to_rec({
          {uri(":source"), uri(sub->source)},
          {uri(":pattern"), uri(sub->pattern)},
          {uri(":qos"), jnt(static_cast<uint8_t>(sub->qos))},
          {uri(":on_recv"), noobj()}
        });
        list.push_back(sub_rec);
      }
      return lst(list);
    }
  };

  using Structure_p = ptr<Structure>;
} // namespace fhatos

#endif
