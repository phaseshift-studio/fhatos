#ifndef fhatos_kernel__local_router_hpp
#define fhatos_kernel__local_router_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/router/router.hpp>
#include <kernel/process/util/mutex/mutex.hpp>

#define FULL_DRAIN_INBOX_SIZE 10

namespace fhatos::kernel {

template <class MESSAGE> class LocalRouter : public Router<MESSAGE> {

protected:
  // messaging data structures
  List<Subscription<MESSAGE>> __SUBSCRIPTIONS;
  Map<ID, MESSAGE> __RETAINS;
  Mutex __MUTEX_SUBSCRIPTION;
  Mutex __MUTEX_RETAIN;

public:
  static LocalRouter *singleton() {
    static LocalRouter singleton = LocalRouter();
    return &singleton;
  }

  LocalRouter(const ID &id = ID("router/local")) : Router<MESSAGE>(id) {}

  virtual const RESPONSE_CODE publish(const MESSAGE &message) override {
    RESPONSE_CODE __rc = RESPONSE_CODE::NO_TARGETS;
    for (const auto &subscription : __SUBSCRIPTIONS) {
      if (subscription.pattern.matches(message.target)) {
        try {
          subscription.actor->push(new Pair<Subscription<MESSAGE>,MESSAGE>(subscription, message));
          // TODO: ACTOR MAILBOX GETTING TOO BIG!
          __rc = RESPONSE_CODE::OK;
        } catch (const std::runtime_error &e) {
          LOG_EXCEPTION(e);
          __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
        }
        // LOG_PUBLISH(__rc ? ERROR : INFO, message);
      }
    }
    if (message.retain) {
      __MUTEX_RETAIN.lockUnlock<void *>([this, message]() {
        __RETAINS.erase(message.target);
        __RETAINS.emplace(message.target, message);
        return nullptr;
      });
    }
    return __rc;
  }

  virtual const RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) override {
    RESPONSE_CODE __rc = RESPONSE_CODE::OK;
    for (const auto &sub : __SUBSCRIPTIONS) {
      if (sub.source.equals(subscription.source) &&
          sub.pattern.equals(subscription.pattern)) {
        __rc = RESPONSE_CODE::REPEAT_SUBSCRIPTION;
        break;
      }
    }
    if (!__rc) {
      try {
        __rc = __MUTEX_SUBSCRIPTION.lockUnlock<RESPONSE_CODE>(
            [this, subscription]() {
              __SUBSCRIPTIONS.push_back(subscription);
              return RESPONSE_CODE::OK;
            });
      } catch (const std::runtime_error &e) {
        LOG_EXCEPTION(e);
        __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
      }
    }
    LOG_SUBSCRIBE(__rc ? ERROR : INFO, subscription);
    ///// deliver retains
    for (const Pair<ID, MESSAGE> &retain : __RETAINS) {
      if (retain.first.matches(subscription.pattern)) {
        subscription.actor->push(new Pair<Subscription<MESSAGE>,MESSAGE>(subscription, retain.second));
      }
    }
    return __rc;
  }

  virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                          const Pattern &pattern) override {
    RESPONSE_CODE __rc;
    try {
      __rc = __MUTEX_SUBSCRIPTION.lockUnlock<RESPONSE_CODE>([this, source,
                                                             pattern]() {
        const uint16_t size = __SUBSCRIPTIONS.size();
        __SUBSCRIPTIONS.remove_if(
            [source, pattern](const Subscription<MESSAGE> &sub) {
              return sub.source.equals(source) && sub.pattern.equals(pattern);
            });
        return __SUBSCRIPTIONS.size() < size ? RESPONSE_CODE::OK
                                             : RESPONSE_CODE::NO_SUBSCRIPTION;
      });
    } catch (const std::runtime_error &e) {
      LOG_EXCEPTION(e);
      __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
    };
    LOG_UNSUBSCRIBE(__rc ? ERROR : INFO, source, pattern);
    return __rc;
  }

  virtual const RESPONSE_CODE unsubscribeSource(const ID &source) override {
    const uint16_t size = __SUBSCRIPTIONS.size();
    List<Subscription<MESSAGE>> __COPY =
        List<Subscription<MESSAGE>>(__SUBSCRIPTIONS);
    for (const Subscription<MESSAGE> &sub : __COPY) {
      if (sub.source.equals(source)) {
        this->unsubscribe(sub.source, sub.pattern);
      }
    }
    return __SUBSCRIPTIONS.size() < size ? RESPONSE_CODE::OK
                                         : RESPONSE_CODE::NO_SUBSCRIPTION;
  }
};
} // namespace fhatos::kernel

#endif