#ifndef fhatos_kernel__publisher_hpp
#define fhatos_kernel__publisher_hpp

#include <kernel/process/actor/mailbox.hpp>
#include <kernel/process/router/message.hpp>
#include <kernel/process/router/router.hpp>

namespace fhatos::kernel {

template <typename ROUTER> class Publisher {

public:
  IDed *ided;
  Mailbox<Mail> *mailbox;
  Publisher(IDed *ided, Mailbox<Mail> *messageBox = nullptr)
      : ided(ided), mailbox(messageBox) {}

  /// SUBSCRIBE
  virtual const RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                        const Consumer<const Message &> onRecv,
                                        const QoS qos = QoS::_1) {
    return ROUTER::singleton()->subscribe(
        Subscription{.mailbox = this->mailbox,
                     .source = this->ided->id(),
                     .pattern = makeTopic(relativePattern),
                     .qos = qos,
                     .onRecv = onRecv});
  }

  /// UNSUBSCRIBE
  virtual const RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
    return ROUTER::singleton()->unsubscribe(this->ided->id(),
                                            makeTopic(relativePattern));
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  /// PUBLISH
  const RESPONSE_CODE publish(const ID &relativeTarget, const Payload &payload,
                              const bool retain = TRANSIENT_MESSAGE) {
    return ROUTER::singleton()->publish(
        Message{.source = this->ided->id(),
                .target = makeTopic(relativeTarget),
                .payload = payload,
                .retain = retain});
  }

  /////////////////////////////////////////////////////////////////////////////////////////

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const bool payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromBool(payload), retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const int payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromInt(payload), retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const long payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromInt((int)payload),
                         retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const float payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromFloat(payload), retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const double payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromFloat((float)payload),
                         retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const char *payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromString(payload), retain);
  }

  inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                     const String &payload,
                                     const bool retain = TRANSIENT_MESSAGE) {
    return this->publish(relativeTarget, Payload::fromString(payload), retain);
  }

private:
  const Pattern makeTopic(const Pattern &relativeTopic) const {
    return relativeTopic.empty()
               ? Pattern(this->ided->id())
               : (relativeTopic.toString().startsWith(F("/"))
                      ? Pattern(this->ided->id().toString() + F("/") +
                                relativeTopic.toString().substring(1))
                      : relativeTopic)
                     .resolve(this->ided->id());
  }
};
} // namespace fhatos::kernel

#endif