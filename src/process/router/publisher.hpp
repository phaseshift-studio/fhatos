#ifndef fhatos_publisher_hpp
#define fhatos_publisher_hpp

#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include <process/router/router.hpp>

namespace fhatos {
  template<typename ROUTER>
  class Publisher {
  public:
    IDed *ided;
    Mailbox<Mail> *mailbox;

    Publisher(IDed *ided, Mailbox<Mail> *messageBox = nullptr)
      : ided(ided), mailbox(messageBox) {
    }

    /// SUBSCRIBE
    virtual const RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                          const Consumer<const Message &> onRecv,
                                          const QoS qos = QoS::_1) {
      return ROUTER::singleton()->subscribe(
        Subscription{
          .mailbox = this->mailbox,
          .source = this->ided->id(),
          .pattern = makeTopic(relativePattern),
          .qos = qos,
          .onRecv = onRecv
        });
    }

    /// UNSUBSCRIBE
    virtual const RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
      return ROUTER::singleton()->unsubscribe(this->ided->id(),
                                              makeTopic(relativePattern));
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /// PUBLISH
    const RESPONSE_CODE publish(const ID &relativeTarget, const BinaryObj<> &payload,
                                const bool retain = TRANSIENT_MESSAGE) {
      return ROUTER::singleton()->publish(
        Message{
          .source = this->ided->id(),
          .target = makeTopic(relativeTarget),
          .payload = payload,
          .retain = retain
        });
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const bool payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(payload), retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const int payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(payload), retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const long payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>((int) payload),
                           retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const float payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(payload), retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const double payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>((float) payload),
                           retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const char *payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(string(payload)), retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const string &payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(payload), retain);
    }

    inline const RESPONSE_CODE publish(const ID &relativeTarget,
                                       const String &payload,
                                       const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, BinaryObj<>(string(payload.c_str(), payload.length())), retain);
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
} // namespace fhatos

#endif
