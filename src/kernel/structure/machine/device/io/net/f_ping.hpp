#ifndef fhatos_kernel__f_ping_hpp
#define fhatos_kernel__f_ping_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <ESPping.h>

namespace fhatos::kernel {

template <typename PROCESS = Fiber, typename ROUTER = LocalRouter<>>
class fPing : public Actor<PROCESS, ROUTER> {
public:
  explicit fPing(const ID &id = fWIFI::idFromIP("ping"))
      : Actor<PROCESS, ROUTER>(id) {}

  ~fPing() {
    this->stop();
    delete this->pingData;
  }

  void setup() override {
    this->subscribe(this->id().extend("#"), [this](const Message &message) {
      if (message.target.subfuri(this->id())) {
        if (message.payloadString().equals("~()")) {
          if (this->pingData) {
            delete this->pingData;
            this->pingData = nullptr;
          }
        } else if (message.payloadString().equals("()")) {
          if (!this->pingData)
            this->pingData = new PingData(message.target.path());
        }
      }
    });
  }
  void loop() override {
    Actor<PROCESS, ROUTER>::loop();
    // 64 bytes from 172.217.12.142: icmp_seq=0 ttl=116 time=87.243 ms
    if (this->pingData) {
      this->pingData->counter++;
      char *message = new char[100];
      if (this->xping.ping(this->pingData->host.c_str(), 1)) {
        this->pingData->success++;
        // this->pingData->totalTime += ::Ping.averageTime();
        sprintf(message, "64 bytes from %s: icmp_seq=%i time=%.3f ms",
                this->pingData->ip.c_str(), this->pingData->counter,
                this->xping.averageTime());

      } else {
        sprintf(message, "Request timeout for icmp_seq %i failure_rate=%.2f%%",
                this->pingData->counter, this->pingData->failureRate());
      }
      this->publish(this->id().extend(this->pingData->host.c_str()), String(message),
                    TRANSIENT_MESSAGE);
      delete[] message;
    }
    delay(1000);
  }

protected:
  struct PingData {
    String host;
    String ip;
    uint16_t counter = 0;
    uint16_t success = 0;
    // float totalTime = 0.0f;

    explicit PingData(const String &host) {
      this->host = host;
      this->ip = fWIFI::resolve(this->host).toString();
    }
    const float failureRate() const {
      return (((float)(counter - success)) / ((float)counter)) * 100.0f;
    }
    // const float successRate() const { return success / counter; }
    // const float averageTime() const { return totalTime / success; }
  };
  ::PingClass xping = ::Ping;
  PingData *pingData =
      nullptr; // make PingData a coroutine and make an array of them
};
} // namespace fhatos::kernel

#endif