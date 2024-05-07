#ifndef fhatos_kernel__f_ping_hpp
#define fhatos_kernel__f_ping_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <ESPping.h>

namespace fhatos::kernel {

struct PingRoutine : public Coroutine {
  String host;
  String ip;
  uint16_t counter = 0;
  uint16_t success = 0;
  // float totalTime = 0.0f;

  explicit PingRoutine(const ID &id) : Coroutine(id) {
    this->host = id.path();
    this->ip = fWIFI::resolve(this->host).toString();
  }
  const float failureRate() const {
    return (((float)(counter - success)) / ((float)counter)) * 100.0f;
  }
};

template <typename PROCESS = Fiber, typename ROUTER = LocalRouter<>>
class fPing : public Actor<PROCESS, ROUTER>, public ParentProcess<PingRoutine> {
public:
  explicit fPing(const ID &id = fWIFI::idFromIP("ping"))
      : Actor<PROCESS, ROUTER>(id), ParentProcess<PingRoutine>() {}

  ~fPing() { this->stop(); }

  void stop() override { this->destroyChildren(); }

  void setup() override {
    this->updateBBS();
    this->subscribe(this->id().extend("#"), [this](const Message &message) {
      if (message.target.subfuri(this->id())) {
        if (message.is<BOOL>()) {
          if (!message.payload.toBool()) {
            this->destroyChildren(message.target);
          } else {
            if (0 == this->searchChildren(Pattern(message.target))) {
              this->_children.emplace_back(new PingRoutine(message.target));
            }
          }
        }
      }
    });
  }

  void loop() override {

    Actor<PROCESS, ROUTER>::loop();
    // 64 bytes from fhatos.org [172.217.12.142]: icmp_seq=0 ttl=116 time=87.243
    // ms
    static int loops = 0;
    if (loops % 10 == 0)
      this->updateBBS();
    for (PingRoutine *pinger : this->_children) {
      pinger->counter++;
      char *message = new char[100];
      if (this->xping.ping(pinger->host.c_str(), 1)) {
        pinger->success++;
        // this->pingData->totalTime += ::Ping.averageTime();
        sprintf(message, "64 bytes from %s [%s]: icmp_seq=%i time=%.3f ms",
                pinger->host.c_str(), pinger->ip.c_str(), pinger->counter,
                this->xping.averageTime());

      } else {
        sprintf(
            message,
            "Request timeout for %s [unknown] icmp_seq %i failure_rate=%.2f%%",
            pinger->host.c_str(), pinger->counter, pinger->failureRate());
      }
      this->publish(this->id().extend(pinger->host.c_str()), message,
                    TRANSIENT_MESSAGE);
      // delete[] message;
    }

    delay(1000);
  }

protected:
  ::PingClass xping = ::Ping;
  void updateBBS() {
    String message = "\n!M!_" + this->id().toString() + "!!\n";
    for (PingRoutine *pinger : this->_children) {
      char line[256];
      sprintf(line,
              FOS_TAB "!b\\_!!!r%s!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                      "[!gip!!:!b%s!!][!gcounter!!:!b%i!!][!gsuccess!!:!b%.2f%%!!]\n",
              pinger->id().toString().c_str(), pinger->ip.c_str(),
              pinger->counter, 100.0f - pinger->failureRate());
      message = message + line;
    }
    this->publish(this->id(), message, RETAIN_MESSAGE);
  }
};
} // namespace fhatos::kernel

#endif