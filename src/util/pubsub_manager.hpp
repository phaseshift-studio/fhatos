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
#ifndef fhatos_pubsub_manager_hpp
#define fhatos_pubsub_manager_hpp

#include <structure/router/router.hpp>
#include <util/mutex_rw.hpp>
#include "structure/pubsub.hpp"

namespace fhatos {

  class PubSubManager {
  protected:
    Function<Mail_p, Option<Mail_p>> _mailer;
    List<Subscription_p> *SUBSCRIPTIONS = new List<Subscription_p>();
    MutexRW<> MUTEX_SUBSCRIPTIONS = MutexRW<>();
    Map<ID, const Message_p> *RETAINS;
    Mutex<> MUTEX_RETAIN = Mutex<>();


  public:
    explicit PubSubManager(
            const bool withRetains = true, const Function<Mail_p, Option<Mail_p>> &mailer = [](const Mail_p &mail) {
      // if (mail->first->mailbox) {
      //    mail->first->mailbox->push(mail);
      //    return Option<Mail_p>();
      //   } else {
      return Option<Mail_p>(mail);
      //   }
    }) : _mailer(withRetains ? mailer : nullptr), RETAINS(withRetains ? new Map<ID, const Message_p>() : nullptr) {}

    ~PubSubManager() {
      this->SUBSCRIPTIONS->clear();
      delete (this->SUBSCRIPTIONS);
      if (RETAINS) {
        RETAINS->clear();
        delete RETAINS;
      }
    }

    RESPONSE_CODE publish(const Message &message) {
      const Message_p mess_ptr = share<Message>(Message{.source = message.source,
              .target = message.target,
              .payload = PtrHelper::clone(message.payload),
              .retain = message.retain});
      auto _rc = MUTEX_SUBSCRIPTIONS.read<Pair<RESPONSE_CODE, List<Subscription_p>>>([this, mess_ptr] {
        //////////////
        RESPONSE_CODE _rc = mess_ptr->retain ? OK : NO_TARGETS;
        List<Subscription_p> subs;
        for (const auto &subscription: *SUBSCRIPTIONS) {
          if (subscription->pattern.matches(mess_ptr->target)) {
            try {
              const Option<Mail_p> mail = this->_mailer(share<Mail>(Mail(subscription, mess_ptr)));
              _rc = OK;
              if (mail.has_value()) {
                subs.push_back(subscription);
              }
            } catch (const fError &e) {
              LOG_EXCEPTION(e);
              _rc = MUTEX_TIMEOUT;
            }
          }
        }
        if (mess_ptr->retain) {
          MUTEX_RETAIN.lockUnlock<void *>([this, mess_ptr]() {
            if (RETAINS->count(mess_ptr->target))
              RETAINS->erase(mess_ptr->target);
            RETAINS->insert({mess_ptr->target, mess_ptr});
            return nullptr;
          });
          LOG(DEBUG, "Total number of retained messages [size:%i]\n", RETAINS->size());
        }
        LOG_PUBLISH(_rc, *mess_ptr);
        return std::make_pair(_rc, subs);
      });
      // process messages of non-threaded subscriptions (prevents mutex dead-lock as this devolves to chained method
      // calls)
      for (const auto &sub: _rc.second) {
        sub->onRecv(mess_ptr);
      }
      MESSAGE_INTERCEPT(message.source, message.target, message.payload, message.retain);
      return _rc.first;
    }

    RESPONSE_CODE subscribe(const Subscription &subscription) {
      try {
        /////////////// SUBSCRIPTION
        RESPONSE_CODE _rc = *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, subscription]() {
          RESPONSE_CODE _rc = OK;
          /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
          SUBSCRIPTIONS->erase(remove_if(SUBSCRIPTIONS->begin(), SUBSCRIPTIONS->end(),
                                         [subscription](const Subscription_p &sub) {
                                           return sub->source.equals(subscription.source) &&
                                                  sub->pattern.equals(subscription.pattern);
                                         }),
                               SUBSCRIPTIONS->end());
          /////////////// ADD NEW SUBSCRIPTION
          const ptr<Subscription> sub_ptr = share<Subscription>(subscription);
          SUBSCRIPTIONS->push_back(sub_ptr);
          LOG_SUBSCRIBE(_rc, sub_ptr);
          return share<RESPONSE_CODE>(_rc);
        });
        /////////////// SUBSCRIPTION RETAINS
        if (RETAINS) {
          for (const auto &mess: MUTEX_RETAIN.lockUnlock<List<Message_p >>([this, subscription] {
            List<Message_p> messages;
            LOG(DEBUG, "Processing retain messages [size:%i]\n", RETAINS->size());
            for (const auto &[target, message]: *RETAINS) {
              if (target.matches(subscription.pattern)) {
                const Option<Mail_p> mail = this->_mailer(share<Mail>(Mail(share(subscription), message)));
                if (mail.has_value()) {
                  messages.push_back(message);
                }
              }
            }
            return messages;
          })) {
            subscription.onRecv(mess);
          }
        }
        return _rc;
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return MUTEX_TIMEOUT;
      }
    }

    RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) { return *unsubscribeX(source, &pattern); }

    RESPONSE_CODE unsubscribeSource(const ID &source) { return *unsubscribeX(source, nullptr); }

    uint retainSize() const { return RETAINS->size(); }

    uint subscriptionSize() const { return SUBSCRIPTIONS->size(); }

    RESPONSE_CODE clear(const bool subscriptions = true, const bool retains = true) {
      if (subscriptions)
        SUBSCRIPTIONS->clear();
      if (retains)
        RETAINS->clear();
      return ((!retains || RETAINS->empty()) && (!subscriptions || SUBSCRIPTIONS->empty())) ? OK : ROUTER_ERROR;
    }

  protected:
    ptr<RESPONSE_CODE> unsubscribeX(const ID &source, const Pattern *pattern) {
      try {
        return MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, source, pattern]() {
          const uint16_t size = SUBSCRIPTIONS->size();
          SUBSCRIPTIONS->erase(remove_if(SUBSCRIPTIONS->begin(), SUBSCRIPTIONS->end(),
                                         [source, pattern](const auto &sub) {
                                           return sub->source.equals(source) &&
                                                  (nullptr == pattern || sub->pattern.equals(*pattern));
                                         }),
                               SUBSCRIPTIONS->end());
          const auto _rc2 =
                  share<RESPONSE_CODE>(((SUBSCRIPTIONS->size() < size) || pattern == nullptr) ? OK : NO_SUBSCRIPTION);
          LOG_UNSUBSCRIBE(*_rc2, source, pattern);
          return _rc2;
        });
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return share<RESPONSE_CODE>(MUTEX_TIMEOUT);
      }
    }
  };
} // namespace fhatos

#endif
