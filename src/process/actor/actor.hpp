#ifndef fhatos_actor_hpp
#define fhatos_actor_hpp

#include <fhatos.hpp>
//
#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include <process/router/meta_router.hpp>
#include <process/router/publisher.hpp>
#include <process/router/router.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {

template <typename PROCESS, typename ROUTER = MetaRouter<>>
class Actor : public PROCESS, public Publisher<ROUTER>, public Mailbox<Mail> {
public:
  explicit Actor(
      const ID &id,
      const Consumer<Actor<PROCESS, ROUTER> *> setupFunction = nullptr,
      const Consumer<Actor<PROCESS, ROUTER> *> loopFunction = nullptr)
      : PROCESS(id), Publisher<ROUTER>(this, this) {
    static_assert(std::is_base_of_v<Process, PROCESS>);
    // static_assert(std::is_base_of_v<Router, ROUTER>);
    if (loopFunction)
      this->onLoop(loopFunction);
    if (setupFunction)
      this->_setupFunction = setupFunction;
  }

  ~Actor() { this->inbox.clear(); }

  static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  String hexStr(unsigned char *data, int len) {
    String s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
      s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
      s[2 * i + 1] = hexmap[data[i] & 0x0F];
    }
    return s;
  }

  const Pair<byte *,uint> serialize() const {
    auto bytes = static_cast<byte *>(malloc(sizeof(*this)));
    memcpy(bytes, static_cast<byte *>(this), sizeof(*this));
    return {bytes,sizeof(*this)};
  }

  static Actor *deserialize(const byte *bytes) {
    return static_cast<Actor *>(bytes);
  }

  // PAYLOAD BOX METHODS
  const bool push(const Mail mail) override {
    return this->running() && this->inbox.push_back(mail);
  }

  const void query(const ID &queryId, const Consumer<const Message>& onRecv) {
    this->publish(queryId.query(emptyString), ("?" + queryId.query()),
                  TRANSIENT_MESSAGE);
    this->subscribe(queryId, [this, queryId, onRecv](const Message& message) {
      if (message.retain) {
        onRecv(message);
        this->unsubscribe(queryId);
      }
    });
  }

  const uint16_t size() const override { return inbox.size(); }

  /// PROCESS METHODS
  virtual void setup() override {
    if (this->_running) {
      LOG(ERROR, "Actor %s has already started [setup()]\n",
          this->id().toString().c_str());
      return;
    }
    ///////////////////////////////////////////////////////
    PROCESS::setup();
    if (this->_setupFunction) {
      this->_setupFunction(this);
    }
  }

  virtual void stop() override {
    if (!this->_running) {
      LOG(ERROR, "Actor %s has already stopped [stop()]\n",
          this->id().toString().c_str());
      return;
    }
    ///////////////////////////////////////////////////////
    if (const RESPONSE_CODE _rc =
        ROUTER::singleton()->unsubscribeSource(this->id())) {
      LOG(ERROR, "Actor %s stop error: %s\n", this->id().toString().c_str(),
          RESPONSE_CODE_STR(_rc).c_str());
    }
    PROCESS::stop();
  }

  virtual void loop() {
    if (!this->_running) {
      LOG(ERROR, "Actor %s has already stopped [loop()]\n",
          this->id().toString().c_str());
      return;
    }
    ///////////////////////////////////////////////////////
    PROCESS::loop();
    if (this->_loopFunction)
      this->_loopFunction(this);
    else {
      while (this->next()) {
      }
    }
  }

  virtual void onLoop(const Consumer<Actor<PROCESS, ROUTER> *> loopFunction) {
    this->_loopFunction = loopFunction;
  }

protected:
  MutexDeque<Mail> inbox;
  Consumer<Actor<PROCESS, ROUTER> *> _setupFunction = nullptr;
  Consumer<Actor<PROCESS, ROUTER> *> _loopFunction = nullptr;
  const Option<Mail> pop() override { return this->inbox.pop_front(); }

  virtual bool next() {
    const Option<Mail> mail = this->pop();
    if (!mail.has_value())
      return false;
    mail->first.execute(mail->second);
    // LOG(INFO, "COUNT for %s: %i", mail->second.payload->toString(),
    // mail->second.payload.use_count());
    // mail->second.payload.data = NULL;
    return true;
  }
};

} // namespace fhatos
#endif