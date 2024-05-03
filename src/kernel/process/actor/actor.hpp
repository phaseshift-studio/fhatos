#ifndef fhatos_kernel__actor_hpp
#define fhatos_kernel__actor_hpp

#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/message_box.hpp>
#include <kernel/process/actor/router/local_router.hpp>
#include <kernel/process/actor/router/meta_router.hpp>
#include <kernel/process/actor/router/mqtt_router.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/process/util/mutex/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos::kernel {
template <typename PROCESS, typename PAYLOAD = String,
          typename ROUTER = MetaRouter<Coroutine, Message<PAYLOAD>>>
class Actor
    : public PROCESS,
      public MessageBox<
          Pair<const Subscription<Message<PAYLOAD>>, const Message<PAYLOAD>>> {
public:
  explicit Actor(const ID &id) : PROCESS(id) {
    static_assert(std::is_base_of_v<Process, PROCESS>);
  }

  /// SUBSCRIBE
  virtual const RESPONSE_CODE
  subscribe(const Pattern &relativePattern,
            const OnRecvFunction<Message<PAYLOAD>> onRecv,
            const QoS qos = QoS::_1) {
    return ROUTER::singleton()->subscribe(
        Subscription<Message<PAYLOAD>>{.actor = this,
                                       .source = this->id(),
                                       .pattern = makeTopic(relativePattern),
                                       .qos = qos,
                                       .onRecv = onRecv});
  }

  /// UNSUBSCRIBE
  virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
    return ROUTER::singleton()->unsubscribe(this->id(),
                                            makeTopic(relativePattern));
  }

  // PUBLISH
  RESPONSE_CODE publish(const IDed &target, const PAYLOAD &message,
                        const bool retain = false) {
    return ROUTER::singleton()->publish(
        Message<PAYLOAD>(this->id(), target.id(), message, retain));
  }

  RESPONSE_CODE publish(const ID &relativeTarget, const PAYLOAD &message,
                        const bool retain = false) {
    return ROUTER::singleton()->publish(Message<PAYLOAD>(
        this->id(), makeTopic(relativeTarget), message, retain));
  }

  // PAYLOAD BOX METHODS
  const bool push(const Pair<const Subscription<Message<PAYLOAD>>,
                             const Message<PAYLOAD>> &mail) override {
    return this->inbox.push_back(mail);
  }

 const uint16_t size() const override { return inbox.size(); }

  /// PROCESS METHODS
  virtual void setup() override { PROCESS::setup(); }

  virtual void stop() override {
    const RESPONSE_CODE _rc =
        ROUTER::singleton()->unsubscribeSource(this->id());
    if (_rc) {
      LOG(ERROR, "Actor %s stop error: %s\n", this->id().toString().c_str(),
          RESPONSE_CODE_STR(_rc).c_str());
    }
    PROCESS::stop();
  }

  virtual void loop() {
    PROCESS::loop();
    //  const long clock = millis();
    //  while ((millis() - clock) < MAX_LOOP_MILLISECONDS) {
    while (this->next()) {
    } // else
    // break;
    //   }
  }

protected:
  MutexDeque<Pair<const Subscription<Message<PAYLOAD>>, const Message<PAYLOAD>>>
      inbox;

  Pattern makeTopic(const Pattern &relativeTopic) {
    return relativeTopic.empty()
               ? Pattern(this->id())
               : (relativeTopic.toString().startsWith(F("/"))
                      ? Pattern(this->id().toString() + F("/") +
                                relativeTopic.toString().substring(1))
                      : relativeTopic);
  }

  Option<Pair<const Subscription<Message<PAYLOAD>>, const Message<PAYLOAD>>>
  pop() override {
    return this->inbox.pop_front();
  }

  virtual bool next() {
    Option<Pair<const Subscription<Message<PAYLOAD>>, const Message<PAYLOAD>>>
        mail = this->pop();
    if (!mail.has_value())
      return false;
    mail->first.execute(mail->second);
    return true;
  }
};

} // namespace fhatos::kernel
#endif