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

#ifndef fhatos_local_router_hpp
#define fhatos_local_router_hpp

#include <fhatos.hpp>
//
#include <process/router/message.hpp>
#include <process/router/router.hpp>
//#include <structure/io/net/f_wifi.hpp>
#include FOS_UTIL(mutex.hpp)
#include <util/mutex_rw.hpp>

#include FOS_PROCESS(coroutine.hpp)

#define NOT_USED 1
#define READING 2
#define WRITING 3

namespace fhatos {
  template<typename PROCESS = Coroutine>
  class LocalRouter : public Router<PROCESS> {
  protected:
    // messaging data structures
    List<ptr<Subscription> > SUBSCRIPTIONS;
    Map<ID, ptr<Message> > RETAINS;
    Mutex<> MUTEX_RETAIN;
    MutexRW<> MUTEX_SUBSCRIPTIONS;

  public:
    static LocalRouter *singleton() {
      static LocalRouter singleton = LocalRouter();
      return &singleton;
    }

    explicit LocalRouter(const ID &id = "kernel/router/local") // fWIFI::idFromIP("kernel", "router/local"))
      : Router<PROCESS>(id) {
    }

    void setup() override {
      PROCESS::setup();
    }

    virtual RESPONSE_CODE clear() override {
      SUBSCRIPTIONS.clear();
      RETAINS.clear();
      return (RETAINS.empty() && SUBSCRIPTIONS.empty())
               ? OK
               : ROUTER_ERROR;
    }

    virtual const RESPONSE_CODE publish(const Message &message) override {
      return MUTEX_SUBSCRIPTIONS.read<RESPONSE_CODE>([this,message]() {
        //////////////
        RESPONSE_CODE _rc = NO_TARGETS;
        for (const auto &subscription: SUBSCRIPTIONS) {
          if (subscription->pattern.matches(message.target)) {
            try {
              if (subscription->mailbox) {
                ptr<Message> message_ptr = share<Message>(Message(message));

                if (!subscription->mailbox->push(share<Mail>(Mail(subscription, message_ptr))))
                  _rc = ROUTER_ERROR;
                else
                  _rc = OK;
              } else {
                subscription->onRecv(message);
                _rc = OK;
              }
              // TODO: ACTOR MAILBOX GETTING TOO BIG!
            } catch (const fError &e) {
              LOG_EXCEPTION(e);
              _rc = MUTEX_TIMEOUT;
            }
            LOG_PUBLISH(_rc, message);
          }
        }

        if (message.retain) {
          MUTEX_RETAIN.lockUnlock<void *>([this, message]() {
            if (RETAINS.count(message.target))
              RETAINS.erase(message.target);
            RETAINS.emplace(message.target, share<Message>(Message(message)));
            return nullptr;
          });
        }
        return _rc;
      });
    }

    const RESPONSE_CODE subscribe(const Subscription &subscription) override {
      try {
        ptr<Subscription> subscription_ptr = share<Subscription>(Subscription(subscription));
        return *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this,subscription_ptr]() {
          RESPONSE_CODE _rc = OK;
          for (const auto &sub: SUBSCRIPTIONS) {
            if (sub->source.equals(subscription_ptr->source) &&
                sub->pattern.equals(subscription_ptr->pattern)) {
              _rc = REPEAT_SUBSCRIPTION;
              break;
            }
          }
          if (!_rc) {
            SUBSCRIPTIONS.push_back(subscription_ptr);
          }
          ///// deliver retains
          if (!_rc) {
            MUTEX_RETAIN.lockUnlock<void *>([this, subscription_ptr]() {
              for (const Pair<const ID, ptr<Message> > &retain: RETAINS) {
                if (retain.first.matches(subscription_ptr->pattern)) {
                  if (subscription_ptr->mailbox) {
                    subscription_ptr->mailbox->push(
                      share<Mail>(Mail(subscription_ptr, retain.second)));
                  } else {
                    subscription_ptr->onRecv(*retain.second);
                  }
                }
              }
              return nullptr;
            });
          }
          LOG_SUBSCRIBE(_rc, subscription_ptr);
          return share<RESPONSE_CODE>(_rc);
        });
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return MUTEX_TIMEOUT;
      }
    }

    const RESPONSE_CODE unsubscribe(const ID &source,
                                    const Pattern &pattern) override {
      return *unsubscribeX(source, &pattern);
    }

    const RESPONSE_CODE unsubscribeSource(const ID &source) override {
      return *unsubscribeX(source, nullptr);
    }

  protected:
    ptr<RESPONSE_CODE> unsubscribeX(const ID &source, const Pattern *pattern) {
      try {
        return MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>(
          [this, source, pattern]() {
            const uint16_t size = SUBSCRIPTIONS.size();
            SUBSCRIPTIONS.erase(remove_if(SUBSCRIPTIONS.begin(), SUBSCRIPTIONS.end(),
                                          [source, pattern](const auto &sub) {
                                            return sub->source.equals(source) &&
                                                   (nullptr == pattern || sub->pattern.equals(*pattern));
                                          }), SUBSCRIPTIONS.end());
            //LOG(INFO, "!bSIZE: %i --> %i \n", SUBSCRIPTIONS.size(), size);
            const auto _rc2 = share<RESPONSE_CODE>((SUBSCRIPTIONS.size() < size)
                                                     ? OK
                                                     : NO_SUBSCRIPTION);
            LOG_UNSUBSCRIBE(*_rc2, source, pattern);
            return _rc2;
          });
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return share<RESPONSE_CODE>(MUTEX_TIMEOUT);
      }
    }
  };
}
#endif
