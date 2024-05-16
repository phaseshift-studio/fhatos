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

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = MetaRouter<> >
  class Actor : public PROCESS, public Publisher<ROUTER>, public Mailbox<Mail> {
  public:
    explicit Actor(
      const ID &id,
      const Consumer<Actor<PROCESS, ROUTER> *> setupFunction = nullptr,
      const Consumer<Actor<PROCESS, ROUTER> *> loopFunction = nullptr)
      : PROCESS(id), Publisher<ROUTER>(this, this) {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      //static_assert(std::is_base_of_v<Router<>, ROUTER>);
      if (loopFunction)
        this->onLoop(loopFunction);
      if (setupFunction)
        this->_setupFunction = setupFunction;
    }

    static constexpr char hexmap[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    String hexStr(const unsigned char *data, const int len) {
      String s(len * 2, ' ');
      for (uint8_t i = 0; i < len; ++i) {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
      }
      return s;
    }

    const Pair<byte *, uint> serialize() const {
      byte *bytes = (byte *) malloc(sizeof(*this));
      memcpy(bytes, (byte *) this, sizeof(*this));
      return {bytes, sizeof(*this)};
    }

    static Actor<PROCESS, ROUTER> *deserialize(const byte *bytes) {
      return (Actor<PROCESS, ROUTER> *) bytes;
    }

    // PAYLOAD BOX METHODS
    bool push(const Mail mail) override {
      return this->running() && this->inbox.push_back(mail);
    }

    void query(const ID &queryId, const Consumer<const Message> &onRecv) {
      this->subscribe(queryId, [this, queryId, onRecv](const Message &message) {
        if (message.retain) {
          this->unsubscribe(queryId);
          onRecv(message);
        }
      });
      this->yield();
      this->loop();
      this->publish(queryId.query(emptyString), ("?" + queryId.query()),
                    TRANSIENT_MESSAGE);
    }

    uint16_t size() const override { return inbox.size(); }

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

    void onQuery(const Pattern pattern, const Consumer<ID> &queryFunction) {
      //this->unsubscribe(pattern);
      this->subscribe(pattern, [this,queryFunction](const Message &message) {
        if (!this->id().equals(message.source))
          queryFunction(message.target);
      });
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

    void onLoop(const Consumer<Actor *> loopFunction) {
      this->_loopFunction = loopFunction;
    }

  protected:
    MutexDeque<Mail> inbox;
    Consumer<Actor *> _setupFunction = nullptr;
    Consumer<Actor *> _loopFunction = nullptr;
    Option<Mail> pop() override { return this->inbox.pop_front(); }

    virtual bool next() {
      const Option<Mail> mail = this->pop();
      if (!mail.has_value())
        return false;
      mail->first.execute(mail->second);
      /// delete mail->second.payload;
      return true;
    }
  };
} // namespace fhatos
#endif
