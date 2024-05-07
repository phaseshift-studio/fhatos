#ifndef fhatos_kernel__local_router_hpp
#define fhatos_kernel__local_router_hpp

#include <fhatos.hpp>
//
#include <kernel/process/router/message.hpp>
#include <kernel/process/router/router.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>
#include <kernel/util/mutex.hpp>

#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {

template <typename PROCESS = Coroutine>
class LocalRouter : public Router<PROCESS> {

protected:
  // messaging data structures
  List<Subscription> SUBSCRIPTIONS;
  Map<ID, Message> RETAINS;
  Mutex MUTEX_SUBSCRIPTION;
  Mutex MUTEX_RETAIN;

public:
  static LocalRouter *singleton() {
    static LocalRouter singleton = LocalRouter();
    return &singleton;
  }

  explicit LocalRouter(const ID &id = fWIFI::idFromIP("kernel", "router/local"))
      : Router<PROCESS>(id) {}

  virtual RESPONSE_CODE clear() override {
    RETAINS.clear();
    SUBSCRIPTIONS.clear();
    return (RETAINS.empty() && SUBSCRIPTIONS.empty())
               ? RESPONSE_CODE::OK
               : RESPONSE_CODE::ROUTER_ERROR;
  }

  virtual const RESPONSE_CODE publish(const Message &message) override {
    RESPONSE_CODE __rc = RESPONSE_CODE::NO_TARGETS;
    for (const auto &subscription : SUBSCRIPTIONS) {
      if (subscription.pattern.matches(message.target)) {
        try {
          if (subscription.mailbox) {
            subscription.mailbox->push(Mail(subscription, message));
          } else {
            subscription.onRecv(message);
          }
          // TODO: ACTOR MAILBOX GETTING TOO BIG!
          __rc = RESPONSE_CODE::OK;
        } catch (const std::runtime_error &e) {
          LOG_EXCEPTION(e);
          __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
        }
        LOG_PUBLISH(__rc ? ERROR : INFO, message);
      }
    }
    if (message.retain) {
      MUTEX_RETAIN.lockUnlock<void *>([this, message]() {
        RETAINS.erase(message.target);
        RETAINS.emplace(message.target, Message(message));
        return nullptr;
      });
    }
    return __rc;
  }

  virtual const RESPONSE_CODE
  subscribe(const Subscription &subscription) override {
    RESPONSE_CODE __rc = RESPONSE_CODE::OK;
    for (const auto &sub : SUBSCRIPTIONS) {
      if (sub.source.equals(subscription.source) &&
          sub.pattern.equals(subscription.pattern)) {
        __rc = RESPONSE_CODE::REPEAT_SUBSCRIPTION;
        break;
      }
    }
    if (!__rc) {
      try {
        __rc = MUTEX_SUBSCRIPTION.lockUnlock<RESPONSE_CODE>(
            [this, subscription]() {
              SUBSCRIPTIONS.push_back(subscription);
              return RESPONSE_CODE::OK;
            });
      } catch (const std::runtime_error &e) {
        LOG_EXCEPTION(e);
        __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
      }
    }
    LOG_SUBSCRIBE(__rc ? ERROR : INFO, subscription);
    ///// deliver retains
    for (const Pair<ID, Message> &retain : RETAINS) {
      if (retain.first.matches(subscription.pattern)) {
        if (subscription.mailbox) {
          subscription.mailbox->push(Mail(subscription, retain.second));
        } else {
          subscription.onRecv(retain.second);
        }
      }
    }
    return __rc;
  }

  const RESPONSE_CODE unsubscribe(const ID &source,
                                  const Pattern &pattern) override {
    return unsubscribeX(source, &pattern);
  }

  const RESPONSE_CODE unsubscribeSource(const ID &source) override {
    return unsubscribeX(source, nullptr);
  }

protected:
  const RESPONSE_CODE unsubscribeX(const ID &source, const Pattern *pattern) {
    RESPONSE_CODE __rc;
    try {
      __rc = MUTEX_SUBSCRIPTION.lockUnlock<RESPONSE_CODE>(
          [this, source, pattern]() {
            const uint16_t size = SUBSCRIPTIONS.size();
            SUBSCRIPTIONS.remove_if([source, pattern](const auto &sub) {
              return sub.source.equals(source) &&
                     (nullptr == pattern || sub.pattern.equals(*pattern));
            });
            return SUBSCRIPTIONS.size() < size ? RESPONSE_CODE::OK
                                               : RESPONSE_CODE::NO_SUBSCRIPTION;
          });
    } catch (const std::runtime_error &e) {
      LOG_EXCEPTION(e);
      __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
    };
    LOG_UNSUBSCRIBE(__rc ? ERROR : INFO, source, pattern);
    return __rc;
  }
};
} // namespace fhatos::kernel

#endif