#ifndef fhatos_kernel__local_broker_hpp
#define fhatos_kernel__local_broker_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/broker/broker.hpp>
#include <kernel/process/actor/messenger.hpp>
#include <kernel/process/util/mutex/mutex.hpp>

#define FULL_DRAIN_INBOX_SIZE 10

namespace fatpig {

template <class MESSAGE> class LocalBroker : public Broker<MESSAGE> {

protected:
  // messaging data structures
  List<Subscription<MESSAGE>> __SUBSCRIPTIONS;
  Map<ID, MESSAGE> __RETAINS;
  Mutex __MUTEX_SUBSCRIPTION;
  Mutex __MUTEX_RETAIN;

public:
  inline static LocalBroker *singleton() {
    static LocalBroker singleton = LocalBroker();
    return &singleton;
  }

  LocalBroker(const ID id = ID("broker/local")) : Broker<MESSAGE>(id) {}

  virtual RESPONSE_CODE publish(const MESSAGE &message) override {
    RESPONSE_CODE __rc = RESPONSE_CODE::NO_TARGETS;
    for (const Subscription<MESSAGE> &subscription : __SUBSCRIPTIONS) {
      if (subscription.pattern.match(message.target)) {
        try {
          subscription.actor->receive({message, subscription});
          __rc = RESPONSE_CODE::OK;
        } catch (const std::runtime_error &e) {
          LOG_EXCEPTION(e);
          __rc = RESPONSE_CODE::MUTEX_TIMEOUT;
        }
        LOG_PUBLISH(__rc ? ERROR : INFO, message);
      }
    }
    if (message.retain) {
      __MUTEX_RETAIN.execute<void *>([this, message]() {
        __RETAINS.erase(message.target);
        __RETAINS.emplace(message.target, message);
        return nullptr;
      });
    }
    return __rc;
  }

  virtual RESPONSE_CODE
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
        __rc =
            __MUTEX_SUBSCRIPTION.execute<RESPONSE_CODE>([this, subscription]() {
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
      if (retain.first.match(subscription.pattern)) {
        subscription.actor->receive({retain.second, subscription});
      }
    }
    return __rc;
  }

  virtual RESPONSE_CODE unsubscribe(const ID &source,
                                    const Pattern &pattern) override {
    RESPONSE_CODE __rc;
    try {
      __rc = __MUTEX_SUBSCRIPTION.execute<RESPONSE_CODE>([this, source,
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

    LOG(__rc ? ERROR : INFO, "[!B%s!!] =!munsubscribe!!=> [!B%s!!]\n",
        source.c_str(), pattern.c_str());
    return __rc;
  }

  virtual RESPONSE_CODE unsubscribeSource(const ID &source) override {
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
} // namespace fatpig

#endif