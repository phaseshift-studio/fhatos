#ifndef fhatos_f_ping_hpp
#define fhatos_f_ping_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <ESPping.h>

namespace fhatos {
  template<typename PROCESS = Coroutine, typename ROUTER = FOS_DEFAULT_ROUTER>
  struct PingRoutine : public Coroutine, public Publisher<ROUTER> {
    String ip;
    uint16_t counter = 0;
    uint16_t success = 0;
    float totalTime = 0.0f;

    explicit PingRoutine(const ID &id) : Coroutine(id), Publisher<ROUTER>(this) {
      this->ip = fWIFI::resolve(this->id().path()).toString();
    }

    void loop() override {
      Coroutine::loop();
      this->counter++;
      char message[100];
      if (Ping.ping(this->id().path().c_str(), 1)) {
        ++this->success;
        this->totalTime += Ping.averageTime();
        sprintf(message, "64 bytes from %s [%s]: icmp_seq=%i time=%.3f ms",
                this->id().path().c_str(), this->ip.c_str(), this->counter,
                this->averageTime());
      } else {
        sprintf(
          message,
          "Request timeout for %s [unknown] icmp_seq %i failure_rate=%.2f%%",
          this->id().path().c_str(), this->counter, this->failureRate());
      }
      this->publish(this->id(), message, TRANSIENT_MESSAGE);
    }

    const float failureRate() const {
      return (static_cast<float>(counter - success) / static_cast<float>(counter)) * 100.0f;
    }

    const float averageTime() const {
      return this->totalTime / (float) this->counter;
    }
  };

  template<typename PROCESS = Coroutine, typename ROUTER = FOS_DEFAULT_ROUTER>
  class fPing : public Actor<PROCESS, ROUTER>,
                public ParentProcess<PingRoutine<ROUTER> > {
  public:
    explicit fPing(const ID &id = fWIFI::idFromIP("ping"))
      : Actor<PROCESS, ROUTER>(id), ParentProcess<PingRoutine<ROUTER> >() {
    }

    void stop() override {
      this->destroyChildren();
      Actor<PROCESS, ROUTER>::stop();
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->updateBBS(this->id().query("out"));
      // handle queries
      this->subscribe(this->id(), [this](const Message &message) {
        if (message.payload->toStr().value() == "?out")
          this->updateBBS(message.target.query("out"));
      });
      // spawn/destroy children
      this->subscribe(this->id().extend("+"), [this](const Message &message) {
        if (message.is<BOOL>()) {
          if (!message.payload->toBool().value()) {
            this->destroyChildren(message.target);
          } else {
            if (0 == this->searchChildren(Pattern(message.target)))
              this->spawnChild(new PingRoutine<ROUTER>(message.target));
          }
          this->updateBBS(message.target.query("out"));
        }
      });
    }

    void loop() override {
      Actor<PROCESS, ROUTER>::loop();
      this->loopChildren();
      delay(1000);
    }

  protected:
    void updateBBS(const ID &queryId) {
      String message = "\n!M!_" + this->id().toString() + "!!\n";
      for (PingRoutine<ROUTER> *pinger: this->_children) {
        char line[256];
        sprintf(line,
                FOS_TAB "!b\\_!!!r%s!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                "[!gip!!:!b%s!!][!gcounter!!:!b%i!!][!gsuccess!!:!b%.2f%%"
                "!!][!gaverage!!:!b%.2fms!!]\n",
                pinger->id().lastSegment().c_str(), pinger->ip.c_str(),
                pinger->counter, 100.0f - pinger->failureRate(),
                pinger->averageTime());
        message = message + line;
      }
      this->publish(queryId, message, RETAIN_MESSAGE);
    }
  };
} // namespace fhatos

#endif
