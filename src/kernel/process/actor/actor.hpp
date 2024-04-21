#ifndef fhatos_kernel__actor_hpp
#define fhatos_kernel__actor_hpp

#include <kernel/process/actor/broker/broker.hpp>
#include <kernel/process/actor/message_box.hpp>
#include <kernel/process/util/mutex/mutex_deque.hpp>

namespace fhatos::kernel {
template <typename TASK, typename MESSAGE, typename BROKER, typename M>
class Actor : public TASK,
              public MessageBox<Pair<Subscription<MESSAGE>, MESSAGE>> {
public:
  Actor(const ID &id) : TASK(id) {}
  virtual RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                  const OnRecvFunction<MESSAGE> onRecv,
                                  const QoS qos = QoS::_1) {
    return BROKER::singleton()->subscribe(
        Subscription<MESSAGE>{.actor = this,
                              .source = this->id(),
                              .pattern = Pattern(makeTopic(relativePattern)),
                              .qos = qos,
                              .onRecv = onRecv});
  }

  virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern = F("")) {
    return BROKER::singleton()->unsubscribe(
        this->id(), Pattern(this->makeTopic(relativePattern)));
  }

  virtual RESPONSE_CODE publish(const ID &relativeTarget, const M &message,
                                const bool retain = RETAIN_MESSAGE) {
    return BROKER::singleton()->publish(
        MESSAGE{.source = this->id(),
                .target = ID(makeTopic(relativeTarget)),
                .payload = message,
                .retain = retain});
  }

  virtual void stop() override {
    const RESPONSE_CODE __rc =
        BROKER::singleton()->unsubscribeSource(this->id());
    if (__rc) {
      LOG(ERROR, "Actor %s stop error: %s\n", this->id().toString().c_str(),
          RESPONSE_CODE_STR(__rc).c_str());
    }
    TASK::stop();
  }

  virtual void start() override {}

  virtual void loop() {
    //  const long clock = millis();
    //  while ((millis() - clock) < MAX_LOOP_MILLISECONDS) {
    while (true) {
      Option<Pair<Subscription<MESSAGE>, MESSAGE>> pair =
          this->inbox.pop_front();
      if (pair.has_value()) {
        LOG_RECEIVE(INFO, pair->second, pair->first);
        pair->first.execute(pair->second);
      } else {
        break;
      }
    } // else
    // break;
    //   }
  }

  virtual Option<Pair<Subscription<MESSAGE>, MESSAGE>> pop() override {
    return this->inbox.pop_front();
  }

  virtual bool push(const Pair<Subscription<MESSAGE>, MESSAGE> &mail) override {
    this->inbox.push_back(mail);
    return true;
  }
  virtual bool next() {
    Option<Pair<Subscription<MESSAGE>, MESSAGE>> mail = this->pop();
    if (!mail.has_value())
      return false;
    mail->first.execute(mail->second);
    return true;
  }
  /*virtual void loop()  {
    while (this->next()) {
    }
  }*/

  virtual uint16_t size() const override { return inbox.size(); }

protected:
  MutexDeque<Pair<Subscription<MESSAGE>, MESSAGE>> inbox;
  const String makeTopic(const fURI &relativeTopic) {
    return relativeTopic.empty()
               ? this->id().toString()
               : (relativeTopic.toString().startsWith(F("/"))
                      ? (this->id().toString() + F("/") +
                         relativeTopic.toString().substring(1))
                      : relativeTopic.toString());
  }
};

} // namespace fhatos::kernel
#endif