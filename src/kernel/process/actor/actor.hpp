#ifndef fhatos_kernel__actor_hpp
#define fhatos_kernel__actor_hpp

#include <fhatos.hpp>
//
#include <kernel/process/actor/message_box.hpp>
#include <kernel/process/router/message.hpp>
#include <kernel/process/router/meta_router.hpp>
#include <kernel/process/router/router.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos::kernel {
template <typename PROCESS, typename ROUTER = MetaRouter<>>
class Actor : public PROCESS,
              public MessageBox<Pair<const Subscription, const Message>> {
public:
  explicit Actor(const ID &id) : PROCESS(id) {
    static_assert(std::is_base_of_v<Process, PROCESS>);
  }

  /// SUBSCRIBE
  virtual const RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                        const Consumer<Message> onRecv,
                                        const QoS qos = QoS::_1) {
    return ROUTER::singleton()->subscribe(
        Subscription{.actor = this,
                     .source = this->id(),
                     .pattern = makeTopic(relativePattern),
                     .qos = qos,
                     .onRecv = onRecv});
  }

  /// UNSUBSCRIBE
  virtual const RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
    return ROUTER::singleton()->unsubscribe(this->id(),
                                            makeTopic(relativePattern));
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  /// PUBLISH
  const RESPONSE_CODE publish(const ID &relativeTarget, const Payload &payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return ROUTER::singleton()->publish(
        Message{.source = this->id(),
                .target = makeTopic(relativeTarget),
                .payload = payload,
                .retain = retain});
  }

  /////////////////////////////////////////////////////////////////////////////////////////

  const RESPONSE_CODE publish(const ID &relativeTarget, const bool payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromBool(payload), retain);
  }

  const RESPONSE_CODE publish(const ID &relativeTarget, const int payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromInt(payload), retain);
  }

  const RESPONSE_CODE publish(const ID &relativeTarget, const long payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromInt((int)payload),
                         retain);
  }

  const RESPONSE_CODE publish(const ID &relativeTarget, const char *payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromString(payload), retain);
  }

  const RESPONSE_CODE publish(const ID &relativeTarget, const String &payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromString(payload), retain);
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  // PAYLOAD BOX METHODS
  const bool
  push(const Pair<const Subscription, const Message> &mail) override {
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
  MutexDeque<Pair<const Subscription, const Message>> inbox;

  Pattern makeTopic(const Pattern &relativeTopic) {
    return relativeTopic.empty()
               ? Pattern(this->id())
               : (relativeTopic.toString().startsWith(F("/"))
                      ? Pattern(this->id().toString() + F("/") +
                                relativeTopic.toString().substring(1))
                      : relativeTopic);
  }

  Option<Pair<const Subscription, const Message>> pop() override {
    return this->inbox.pop_front();
  }

  virtual bool next() {
    Option<Pair<const Subscription, const Message>> mail = this->pop();
    if (!mail.has_value())
      return false;
    mail->first.execute(mail->second);
    // delete mail->second.payload.data;
    return true;
  }
};

} // namespace fhatos::kernel
#endif