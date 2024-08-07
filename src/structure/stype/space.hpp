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
#ifndef fhatos_space_hpp
#define fhatos_space_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/rooter.hpp>
#include <structure/structure.hpp>

namespace fhatos {

  class Space : public Structure {

  protected:
    Map<ID, const Message_p> *DATA;
    MutexRW<> MUTEX_DATA = MutexRW<>();
    MutexDeque<Mail_p> *OUTGOING;
    List<Subscription_p> *SUBSCRIPTIONS = new List<Subscription_p>();
    MutexRW<> MUTEX_SUBSCRIPTIONS = MutexRW<>();

    explicit Space(const Pattern_p &pattern) :
        Structure(pattern, SType::READWRITE), DATA(new Map<ID, const Message_p>()), MUTEX_DATA(MutexRW<>()),
        OUTGOING(new MutexDeque<Mail_p>()) {}

  public:
    static Space *create(const Pattern_p &pattern) { return new Space(pattern); }

    ~Space() {
      delete DATA;
      delete OUTGOING;
    }

    void close() override {
      Structure::close();
      DATA->clear();
    }

    void write(const Message_p &message) override {
      Structure::write(message);
      if (message->retain) {
        MUTEX_DATA.write<Obj>([this, message]() {
          Obj_p ret = Obj::to_noobj();
          if (DATA->count(message->target)) {
            ret = DATA->at(message->target)->payload;
            DATA->erase(message->target);
          }
          DATA->insert({message->target, message});
          return ret;
        });
      }
      MUTEX_SUBSCRIPTIONS.read<void *>([this, message]() {
        for (const auto &subscription: *SUBSCRIPTIONS) {
          if (subscription->pattern.matches(message->target)) {
            const Mail_p mail = share(Mail{subscription, message});
            if (subscription->mailbox) {
              subscription->mailbox->push(mail);
            } else {
              OUTGOING->push_back(mail);
            }
          }
        }
        return nullptr;
      });
    }

    void maintain() override {
      Structure::maintain();
      Mail_p mail;
      if ((mail = OUTGOING->pop_back().value_or(nullptr))) {
        mail->first->onRecv(mail->second);
      }
    }

    void read(const Subscription_p &subscription) override {
      Structure::read(subscription);
      MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, subscription]() {
        RESPONSE_CODE _rc = OK;
        /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
        SUBSCRIPTIONS->erase(remove_if(SUBSCRIPTIONS->begin(), SUBSCRIPTIONS->end(),
                                       [subscription](const Subscription_p &sub) {
                                         return sub->source.equals(subscription->source) &&
                                                sub->pattern.equals(subscription->pattern);
                                       }),
                             SUBSCRIPTIONS->end());
        /////////////// ADD NEW SUBSCRIPTION
        SUBSCRIPTIONS->push_back(subscription);
        LOG_SUBSCRIBE(_rc, subscription);
        return share<RESPONSE_CODE>(_rc);
      });
      MUTEX_DATA.read<void *>([this, subscription]() {
        for (const auto &[id, message]: *DATA) {
          if (id.matches(subscription->pattern)) {
            const Mail_p mail = share(Mail{subscription, message});
            if (subscription->mailbox) {
              subscription->mailbox->push(mail);
            } else {
              OUTGOING->push_back(mail);
            }
          }
        }
        return nullptr;
      });
    }
  };
} // namespace fhatos

#endif
