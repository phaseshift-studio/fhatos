#ifndef fhatos_kernel__actor_hpp
#define fhatos_kernel__actor_hpp

#include <kernel/process/actor/broker/broker.hpp>
#include <kernel/process/actor/messenger.hpp>
#include <kernel/process/util/mutex/mutex_deque.hpp>

namespace fhatos::kernel {
template <typename TASK, typename MESSAGE, typename BROKER>
class Actor : public IDed, public Messenger<MESSAGE> {
public:
  Actor(const ID &id) : IDed(id) {}
  virtual RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                  const OnRecvFunction<MESSAGE> onRecv,
                                  const QoS qos = QoS::_1) {
    return BROKER::singleton()->subscribe(
        Subscription<MESSAGE>{.actor = this,
                              .source = this->id(),
                              .pattern = makeTopic(relativePattern),
                              .qos = qos,
                              .onRecv = onRecv});
  }

  virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern = F("")) {
    return BROKER::singleton()->unsubscribe(this->id(),
                                            this->makeTopic(relativePattern));
  }

  virtual RESPONSE_CODE publish(const ID &relativeTarget, const M &message,
                                const bool retain = RETAIN_MESSAGE) {
    return BROKER::singleton()->publish(
        MESSAGE{.source = this->id(),
                .target = makeTopic(relativeTarget),
                .payload = message,
                .retain = retain});
  }

  virtual void stop() override {
    const RESPONSE_CODE __rc =
        BROKER::singleton()->unsubscribeSource(this->id());
    if (__rc) {
      LOG(ERROR, "Actor %s stop error: %s\n", this->id().c_str(),
          RESPONSE_CODE_STR(__rc).c_str());
    }
    Task<T>::stop();
  }

  virtual void loop() override {
    //  const long clock = millis();
    //  while ((millis() - clock) < MAX_LOOP_MILLISECONDS) {
    Option<Pair<MESSAGE, Subscription<MESSAGE>>> pair = this->inbox.pop_front();
    if (pair.has_value()) {
      LOG_RECEIVE(INFO, pair->first, pair->second);
      pair->second.execute(pair->first);
    } // else
    // break;
    //   }
  }

  virtual const Option<Pair<Subcription<MESSAGE>, Message<MESSAGE>>> pop();
  virtual bool push(const Pair<Subscription<MESSAGE>, Message<MESSAGE>> &mail);
  virtual bool next() {
    const Option<Pair<Subscription<MESSAGE>, Message<MESSAGE>>> &mail =
        this->pop();
    if (mail.empty())
      return false;
    mail.get().first.execute(mail.get().second);
    return true;
  }
  virtual void loop() override {
    while (this->next()) {
    }
  }

protected:
  MutexDeque<Pair<MESSAGE, Subscription<MESSAGE>>> inbox;
  const Pattern makeTopic(const Pattern &relativeTopic) {
    return relativeTopic.isEmpty() ? Pattern(this->id())
                                   : (relativeTopic.startsWith(F("/"))
                                          ? Pattern(this->id() + F("/") +
                                                    relativeTopic.substring(1))
                                          : relativeTopic);
  }
};

} // namespace fhatos::kernel
#endif