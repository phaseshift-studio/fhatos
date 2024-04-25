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
          typename ROUTER = MetaRouter<MESSAGE>>
class Actor : public PROCESS,
              public MessageBox<Pair<const Subscription<MESSAGE>, const MESSAGE>> {
public:
  Actor(const ID &id) : PROCESS(id) {
    static_assert(std::is_base_of<Router<MESSAGE>, ROUTER>::value,
                  "ROUTER not derived from Router<MESSAGE>");
  }
  /// SUBSCRIBE
  virtual const RESPONSE_CODE subscribe(const Pattern &relativePattern,
                                        const OnRecvFunction<MESSAGE> onRecv,
                                        const QoS qos = QoS::_1) {
    return ROUTER::singleton()->subscribe(
        Subscription<MESSAGE>{.actor = this,
                              .source = this->id(),
                              .pattern = makeTopic(relativePattern),
                              .qos = qos,
                              .onRecv = onRecv});
  }

  /// UNSUBSCRIBE
  virtual const RESPONSE_CODE
  unsubscribe(const Pattern &relativePattern = F("")) {
    return ROUTER::singleton()->unsubscribe(this->id(),
                                            makeTopic(relativePattern));
  }

  // PUBLISH
  const RESPONSE_CODE publish(const IDed &target, const String &message,
                              const bool retain = TRANSIENT_MESSAGE) {
    return ROUTER::singleton()->publish(
        MESSAGE(this->id(), target.id(), message, retain));
  }
  virtual const RESPONSE_CODE publish(const ID &relativeTarget,
                                      const String &message,
                                      const bool retain = TRANSIENT_MESSAGE) {
    return ROUTER::singleton()->publish(
        MESSAGE(this->id(), makeTopic(relativeTarget), message, retain));
  }

  // MESSAGE BOX METHODS
  virtual const bool push(const Pair<const Subscription<MESSAGE>, const MESSAGE>& mail) override {
    return this->inbox.push_back(mail);
  }

  virtual const uint16_t size() const override { return inbox.size(); }

  /// PROCESS METHODS
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
    while (this->next()) {
    } // else
    // break;
    //   }
  }

protected:
  MutexDeque<Pair<const Subscription<MESSAGE>, const MESSAGE>> inbox;
  const Pattern makeTopic(const Pattern &relativeTopic) {
    return relativeTopic.empty()
               ? Pattern(this->id())
               : (relativeTopic.toString().startsWith(F("/"))
                      ? Pattern(this->id().toString() + F("/") +
                                relativeTopic.toString().substring(1))
                      : relativeTopic);
  }
  virtual Option<Pair<const Subscription<MESSAGE>, const MESSAGE>> pop() override {
    return this->inbox.pop_front();
  }

  virtual bool next() {
    Option<Pair<const Subscription<MESSAGE>, const MESSAGE>> mail = this->pop();
    if (!mail.has_value())
      return false;
    mail->first.execute(mail->second);
    return true;
  }
};

} // namespace fhatos::kernel
#endif