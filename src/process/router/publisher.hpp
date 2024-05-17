#ifndef fhatos_publisher_hpp
#define fhatos_publisher_hpp

#include <process/actor/mailbox.hpp>
#include <process/esp32/scheduler.hpp>
#include <process/router/message.hpp>
#include <process/router/router.hpp>

namespace fhatos {
  template<typename ROUTER>
  class Publisher {
  public:
    IDed *ided;
    Mailbox<Mail> *mailbox;

    explicit Publisher(IDed *ided, Mailbox<Mail> *mailbox = nullptr)
      : ided(ided), mailbox(mailbox) {
    }

    /// SUBSCRIBE
    virtual RESPONSE_CODE subscribe(const Pattern &relativePattern,
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

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const Runnable &runnable) {
      this->subscribe(queryPattern, [queryPattern,runnable](const Message &message) {
        if (!message.isReflexive() && message.isQuery(queryPattern.query().c_str()))
          runnable();
      });
      return this;
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const BiConsumer<SourceID, TargetID> &consumer) {
      this->subscribe(queryPattern, [queryPattern,consumer](const Message &message) {
        if (!message.isReflexive() && message.isQuery(queryPattern.query().c_str()))
          consumer(message.source, message.target);
      });
      return this;
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern,
                               const Map<string, BiConsumer<SourceID, TargetID> > &mapping,
                               const BiConsumer<SourceID, TargetID> &thenDo = nullptr) {
      this->subscribe(queryPattern, [mapping,thenDo](const Message &message) {
        if (message.target.hasQuery() && !message.isReflexive()) {
          if (const string query = message.target.query(); mapping.count(query)) {
            mapping.at(query)(message.source, message.target);
          } else if (mapping.count("default")) {
            mapping.at("default")(message.source, message.target);
          }
          if (thenDo)
            thenDo(message.source, message.target);
        }
      });
      return this;
    }

    /// UNSUBSCRIBE
    virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
      return ROUTER::singleton()->unsubscribe(this->ided->id(),
                                              makeTopic(relativePattern));
    }

    virtual RESPONSE_CODE unsubscribeSource() {
      return ROUTER::singleton()->unsubscribeSource(this->ided->id());
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /// PUBLISH
    inline RESPONSE_CODE publish(const ID &relativeTarget, const BinaryObj<> *payload,
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

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const bool payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const int payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const long payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>((int) payload),
                           retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const float payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const double payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>((float) payload),
                           retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const char *payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(string(payload)), retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const string &payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    inline RESPONSE_CODE publish(const ID &relativeTarget,
                                 const String &payload,
                                 const bool retain = TRANSIENT_MESSAGE) {
      return this->publish(relativeTarget, new BinaryObj<>(string(payload.c_str(), payload.length())), retain);
    }

  private:
     Pattern makeTopic(const Pattern &relativeTopic) const {
      return relativeTopic.empty()
               ? Pattern(this->ided->id())
               : (relativeTopic.toString()[0] == '/'
                    ? Pattern(this->ided->id().toString() + "/" +
                              relativeTopic.toString().substr(1))
                    : relativeTopic)
               .resolve(this->ided->id());
    }
  };
} // namespace fhatos

#endif
