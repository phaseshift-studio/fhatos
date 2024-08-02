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
#ifndef fhatos_local_router_hpp
#define fhatos_local_router_hpp

#include <fhatos.hpp>
//
#include <process/router/message.hpp>
#include <process/router/router.hpp>
// #include <structure/io/net/f_wifi.hpp>
#include FOS_UTIL(mutex.hpp)
#include <util/mutex_rw.hpp>

namespace fhatos {
  class LocalRouter final : public Router {

    explicit LocalRouter(const ID &id = ID("/router/local/")) :
        Router(id, ROUTER_LEVEL::LOCAL_ROUTER), MUTEX_RETAIN(id.toString().c_str()),
        MUTEX_SUBSCRIPTIONS(id.toString().c_str()) {}

  protected:
    List<Subscription_p> SUBSCRIPTIONS;
    Map<ID, const Message_p> RETAINS;
    Mutex<> MUTEX_RETAIN;
    MutexRW<> MUTEX_SUBSCRIPTIONS;

  public:
    static LocalRouter *singleton(const ID &id = ID("/router/local/")) {
      static LocalRouter local = LocalRouter(id);
      return &local;
    }

    uint retainSize() const override { return RETAINS.size(); }

    uint subscriptionSize() const { return SUBSCRIPTIONS.size(); }

    RESPONSE_CODE clear() override {
      // SUBSCRIPTIONS.clear();
      RETAINS.clear();
      return (RETAINS.empty() && SUBSCRIPTIONS.empty()) ? OK : ROUTER_ERROR;
    }

    RESPONSE_CODE publish(const Message &message) override {
      const Message_p mess_ptr = share<Message>(Message{.source = message.source,
                                                        .target = message.target,
                                                        .payload = PtrHelper::clone(message.payload),
                                                        .retain = message.retain});
      auto _rc = MUTEX_SUBSCRIPTIONS.read<Pair<RESPONSE_CODE, List<Subscription_p>>>([this, mess_ptr] {
        //////////////
        RESPONSE_CODE _rc = mess_ptr->retain ? OK : NO_TARGETS;
        List<Subscription_p> subs;
        for (const auto &subscription: SUBSCRIPTIONS) {
          if (subscription->pattern.matches(mess_ptr->target)) {
            try {
              if (subscription->mailbox) {
                _rc = subscription->mailbox->push(share<Mail>(Mail(subscription, mess_ptr))) ? OK : ROUTER_ERROR;
                LOG(TRACE, "Message from !b%s!! delivered to mailbox !b%s!!\n", mess_ptr->source.toString().c_str(),
                    subscription->source.toString().c_str());
                if (subscription->mailbox->size() > FOS_MAILBOX_WARNING_SIZE) {
                  LOG(WARN, "Mailbox !b%s!! reached warning size of %i: [size:%i]\n",
                      subscription->source.toString().c_str(), FOS_MAILBOX_WARNING_SIZE, subscription->mailbox->size());
                }
              } else {
                subs.push_back(subscription);
                _rc = OK;
              }
            } catch (const fError &e) {
              LOG_EXCEPTION(e);
              _rc = MUTEX_TIMEOUT;
            }
          }
        }
        if (mess_ptr->retain) {
          MUTEX_RETAIN.lockUnlock<void *>([this, mess_ptr]() {
            if (RETAINS.count(mess_ptr->target))
              RETAINS.erase(mess_ptr->target);
            RETAINS.insert({mess_ptr->target, mess_ptr});
            return nullptr;
          });
          LOG(DEBUG, "Total number of retained messages [size:%i]\n", RETAINS.size());
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

    RESPONSE_CODE subscribe(const Subscription &subscription) override {
      try {
        /////////////// SUBSCRIPTION
        RESPONSE_CODE _rc = *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, subscription]() {
          RESPONSE_CODE _rc = OK;
          /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
          SUBSCRIPTIONS.erase(remove_if(SUBSCRIPTIONS.begin(), SUBSCRIPTIONS.end(),
                                        [subscription](const Subscription_p &sub) {
                                          return sub->source.equals(subscription.source) &&
                                                 sub->pattern.equals(subscription.pattern);
                                        }),
                              SUBSCRIPTIONS.end());
          /////////////// ADD NEW SUBSCRIPTION
          const ptr<Subscription> sub_ptr = share<Subscription>(subscription);
          SUBSCRIPTIONS.push_back(sub_ptr);
          LOG_SUBSCRIBE(_rc, sub_ptr);
          return share<RESPONSE_CODE>(_rc);
        });
        /////////////// SUBSCRIPTION RETAINS
        for (const auto &mess: MUTEX_RETAIN.lockUnlock<List<Message_p>>([this, subscription] {
               List<Message_p> messages;
               LOG(DEBUG, "Processing retain messages [size:%i]\n", RETAINS.size());
               for (const auto &[target, message]: RETAINS) {
                 if (target.matches(subscription.pattern)) {
                   if (subscription.mailbox) {
                     subscription.mailbox->push(share<Mail>(Mail(share(subscription), message)));
                   } else {
                     messages.push_back(message);
                   }
                 }
               }
               return messages;
             })) {
          subscription.onRecv(mess);
        }
        return _rc;
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return MUTEX_TIMEOUT;
      }
    }

    RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return *unsubscribeX(source, &pattern);
    }

    RESPONSE_CODE unsubscribeSource(const ID &source) override { return *unsubscribeX(source, nullptr); }

  protected:
    ptr<RESPONSE_CODE> unsubscribeX(const ID &source, const Pattern *pattern) {
      try {
        return MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, source, pattern]() {
          const uint16_t size = SUBSCRIPTIONS.size();
          SUBSCRIPTIONS.erase(remove_if(SUBSCRIPTIONS.begin(), SUBSCRIPTIONS.end(),
                                        [source, pattern](const auto &sub) {
                                          return sub->source.equals(source) &&
                                                 (nullptr == pattern || sub->pattern.equals(*pattern));
                                        }),
                              SUBSCRIPTIONS.end());
          const auto _rc2 =
              share<RESPONSE_CODE>(((SUBSCRIPTIONS.size() < size) || pattern == nullptr) ? OK : NO_SUBSCRIPTION);
          LOG_UNSUBSCRIBE(*_rc2, source, pattern);
          return _rc2;
        });
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return share<RESPONSE_CODE>(MUTEX_TIMEOUT);
      }
    }

    const string toString() const override { return "LocalRouter"; }
  };
} // namespace fhatos
#endif
