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

#include FOS_PROCESS(coroutine.hpp)

#include <util/obj_helper.hpp>

namespace fhatos {
  class LocalRouter final : public Router {

  private:
    explicit LocalRouter(const ID &id = ID("/router/local/")) : Router(id, ROUTER_LEVEL::LOCAL_ROUTER) {
      this->subscribe(Subscription{
          .source = id, .pattern = id, .qos = QoS::_1, .onRecv = [id](const Message_p &message) {
            LOG(INFO, "LocalRouter %s subscription received: %s\n", id.toString().c_str(), message->toString().c_str());
          }});
    }

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

    const RESPONSE_CODE clear() override {
      // SUBSCRIPTIONS.clear();
      RETAINS.clear();
      return (RETAINS.empty() && SUBSCRIPTIONS.empty()) ? OK : ROUTER_ERROR;
    }

    const RESPONSE_CODE publish(const Message &message) override {
      auto _rc = MUTEX_SUBSCRIPTIONS.read<RESPONSE_CODE>([this, message] {
        //////////////
        RESPONSE_CODE _rc = message.retain ? OK : NO_TARGETS;
        const Message_p mess_ptr = share<Message>(Message{
            .source = message.source, .target = message.target, .payload = message.payload, .retain = message.retain});
        for (const auto &subscription: SUBSCRIPTIONS) {
          if (subscription->pattern.matches(message.target)) {
            try {
              if (subscription->mailbox) {
                _rc = subscription->mailbox->push(share<Mail>(Mail(subscription, mess_ptr))) ? OK : ROUTER_ERROR;
                LOG(DEBUG, "Message from %s delivered to %s\n", message.source.toString().c_str(),
                    subscription->source.toString().c_str());
                if (subscription->mailbox->size() > FOS_MAILBOX_WARNING_SIZE) {
                  LOG(ERROR, "Actor mailbox size is beyond warning size of %i: [size:%i]\n", FOS_MAILBOX_WARNING_SIZE,
                      subscription->mailbox->size());
                }
              } else {
                subscription->onRecv(mess_ptr);
                _rc = OK;
              }

            } catch (const fError &e) {
              LOG_EXCEPTION(e);
              _rc = MUTEX_TIMEOUT;
            }
          }
        }
        if (message.retain) {
          MUTEX_RETAIN.lockUnlock<void *>([this, mess_ptr]() {
            if (RETAINS.count(mess_ptr->target))
              RETAINS.erase(mess_ptr->target);
            RETAINS.insert({mess_ptr->target, mess_ptr});
            return nullptr;
          });
          LOG(DEBUG, "Total number of retained messages [size:%i]\n", RETAINS.size());
        }
        LOG_PUBLISH(_rc, message);
        return _rc;
      });
      OBJ_HANDLER(message.target, message.payload);
      return _rc;
    }

    const RESPONSE_CODE subscribe(const Subscription &subscription) override {
      try {
        /////////////// SUBSCRIPTION
        RESPONSE_CODE _rc = *MUTEX_SUBSCRIPTIONS.write<RESPONSE_CODE>([this, subscription]() {
          RESPONSE_CODE _rc = OK;
          /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
          erase_if(SUBSCRIPTIONS, [subscription](Subscription_p &sub) {
            return sub->source.equals(subscription.source) && sub->pattern.equals(subscription.pattern);
          });
          /////////////// ADD NEW SUBSCRIPTION
          const ptr<Subscription> sub_ptr = share<Subscription>(subscription);
          SUBSCRIPTIONS.push_back(sub_ptr);
          LOG_SUBSCRIBE(_rc, sub_ptr);
          return share<RESPONSE_CODE>(_rc);
        });
        /////////////// SUBSCRIPTION RETAINS
        MUTEX_RETAIN.lockUnlock<void *>([this, subscription]() {
          LOG(DEBUG, "Processing retain messages [size:%i]\n", RETAINS.size());
          for (const auto &[target, message]: RETAINS) {
            if (target.matches(subscription.pattern)) {
              if (subscription.mailbox) {
                subscription.mailbox->push(share<Mail>(Mail(share(subscription), message)));
              } else {
                subscription.onRecv(message);
              }
            }
          }
          return nullptr;
        });
        ////////////////////////////////////////////
        /*Obj::LstList_p<> subs = share(Obj::LstList<>());
        for (Subscription_p s: SUBSCRIPTIONS) {
          subs->push_back(
              Obj::to_rec({{u("source"), u(s->source)}, {u("pattern"), u(s->pattern)}, {u("qos"), (int) s->qos}}));
        }
        this->publish(Message{.source = "/kernel/router/local",
                              .target = "/kernel/router/local/subscription",
                              .payload = Obj::to_lst(subs),
                              .retain = RETAIN_MESSAGE});*/
        return _rc;
      } catch (const fError &e) {
        LOG_EXCEPTION(e);
        return MUTEX_TIMEOUT;
      }
    }

    const RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) override {
      return *unsubscribeX(source, &pattern);
    }

    const RESPONSE_CODE unsubscribeSource(const ID &source) override { return *unsubscribeX(source, nullptr); }

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
