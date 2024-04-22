#ifndef fhatos_kernel__actor_hpp
#define fhatos_kernel__actor_hpp

#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/message_box.hpp>
#include <kernel/process/actor/router/local_router/local_router.hpp>
#include <kernel/process/actor/router/meta_router/meta_router.hpp>
#include <kernel/process/actor/router/mqtt_router/mqtt_router.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/process/util/mutex/mutex_deque.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {
template <typename PROCESS, typename MESSAGE = StringMessage,
          typename ROUTER = MqttRouter<MESSAGE, MQTT<Thread, MESSAGE>>>
class Actor : public PROCESS,
              public MessageBox<Pair<Subscription<MESSAGE>, MESSAGE>> {
public:
  Actor(const ID &id) : PROCESS(id) {
    //this->inbox = new MutexDeque<Pair<Subscription<MESSAGE>, MESSAGE>>();
  }
  virtual const RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                        const OnRecvFunction<MESSAGE> onRecv,
                                        const QoS qos = QoS::_1) {
    return ROUTER::singleton()->subscribe(
        Subscription<MESSAGE>{.actor = this,
                              .source = this->id(),
                              .pattern = Pattern(makeTopic(relativePattern)),
                              .qos = qos,
                              .onRecv = onRecv});
  }

  virtual const RESPONSE_CODE
  unsubscribe(const Pattern &relativePattern = F("")) {
    return ROUTER::singleton()->unsubscribe(
        this->id(), Pattern(this->makeTopic(relativePattern)));
  }

  const RESPONSE_CODE publish(const IDed &target, const MESSAGE &message,
                              const bool retain = RETAIN_MESSAGE) {
    return ROUTER::singleton()->publish(MESSAGE{.source = this->id(),
                                                .target = target.id(),
                                                .payload = message.payload,
                                                .retain = retain});
  }
  virtual const RESPONSE_CODE publish(const ID &relativeTarget,
                                      const MESSAGE &message,
                                      const bool retain = RETAIN_MESSAGE) {
    return ROUTER::singleton()->publish(
        MESSAGE{.source = this->id(),
                .target = ID(makeTopic(relativeTarget)),
                .payload = message.payload,
                .retain = retain});
  }

  virtual void setup() override { PROCESS::setup(); }

  virtual void stop() override {
    const RESPONSE_CODE __rc =
        ROUTER::singleton()->unsubscribeSource(this->id());
    if (__rc) {
      LOG(ERROR, "Actor %s stop error: %s\n", this->id().toString().c_str(),
          RESPONSE_CODE_STR(__rc).c_str());
    }
    PROCESS::stop();
  }

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