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

#pragma once
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/router/router.hpp>
#include <util/enums.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>

namespace fhatos {
  using IDxOBJ = Pair<ID_p, Obj_p>;
  enum class SType { READ, WRITE, READWRITE };
  static const Enums<SType> StructureTypes =
      Enums<SType>({{SType::READ, "read"}, {SType::WRITE, "write"}, {SType::READWRITE, "readwrite"}});

  class Structure : public Patterned {

  protected:
    MutexDeque<Mail_p> *outbox = new MutexDeque<Mail_p>();
    List<Subscription_p> *subscriptions = new List<Subscription_p>();
    MutexRW<> mutex = MutexRW<>();
    std::atomic_bool _available = std::atomic_bool(false);

  public:
    const SType stype;

    explicit Structure(const Pattern &pattern, const SType stype) : Patterned(share(pattern)), stype(stype) {}

    virtual ~Structure() = default;

    bool available() { return this->_available.load(); }
    virtual void setup() {
      if (this->_available.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
      this->_available.store(true);
    }
    virtual void loop() {
      if (!this->_available.load())
        throw fError(FURI_WRAP " !ystructure!! is closed\n", this->pattern()->toString().c_str());
      Option<Mail_p> mail;
      while ((mail = this->outbox->pop_back()).has_value()) {
        mail.value()->first->onRecv(mail.value()->second);
      }
    }
    virtual void stop() {
      if (!this->_available.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already closed");
      this->_available.store(false);
      this->subscriptions->clear();
      this->outbox->clear(false);
    }

    virtual void recv_subscription(const Subscription_p &subscription) {
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
        if (subscription->onRecv) { // not an unsubscribe event
          this->subscriptions->push_back(subscription);
          const List<IDxOBJ> payload = this->read(share(subscription->pattern), subscription->source);
          for (const auto &[furi, obj]: payload) {
            this->outbox->push_back(share(
                Mail({subscription,
                      share(Message{.source = *this->pattern(), .target = *furi, .payload = obj, .retain = true})})));
          }
          LOG_SUBSCRIBE(OK, subscription);
        } else {
          LOG_UNSUBSCRIBE(OK, subscription->source, &subscription->pattern);
        }
        return nullptr;
      });
    }

    virtual void recv_message(const Message_p &message) {
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      if (message->retain) {
        this->write(share(message->target), message->payload, message->source);
      }
      RESPONSE_CODE rc = mutex.read<RESPONSE_CODE>([this, message]() {
        RESPONSE_CODE rc2 = NO_SUBSCRIPTION;
        for (const auto &subscription: *this->subscriptions) {
          if (message->target.matches(subscription->pattern)) {
            rc2 = OK;
            this->outbox->push_back(share(Mail{subscription, message}));
          }
        }
        return rc2;
      });
      LOG_PUBLISH(rc, *message);
    }
    virtual void remove(const ID_p &id, const ID &source) { this->write(id, Obj::to_noobj(), source); }
    virtual Obj_p read(const ID_p &id, const ID &source) = 0;
    virtual Obj_p read(const ID_p &id) { return this->read(id, FOS_DEFAULT_SOURCE_ID); };
    virtual List<IDxOBJ> read(const fURI_p &furi, const ID &source) = 0;
    virtual List<IDxOBJ> read(const fURI_p &furi) { return this->read(furi, FOS_DEFAULT_SOURCE_ID); }
    virtual void write(const ID_p &id, const Obj_p &obj, const ID &source) = 0;
    virtual Obj_p write(const ID_p &id, const Obj_p &obj) { this->write(id, obj, FOS_DEFAULT_SOURCE_ID); }
  };


} // namespace fhatos

#endif
