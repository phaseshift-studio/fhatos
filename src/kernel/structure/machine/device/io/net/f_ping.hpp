#ifndef fhatos_kernel__f_ping_hpp
#define fhatos_kernel__f_ping_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <ESPping.h>

namespace fhatos::kernel {

template <typename PROCESS = Fiber, typename PAYLOAD = String,
          typename ROUTER = LocalRouter<Message<PAYLOAD>>>
class fPing : public Actor<PROCESS, PAYLOAD, ROUTER> {
public:
  explicit fPing(const ID &id = fWIFI::idFromIP("ping"))
      : Actor<PROCESS, PAYLOAD, ROUTER>(id) {}

  ~fPing() { delete this->pingData; }

   void setup() override {
    this->subscribe(this->id(), [this](const Message<PAYLOAD> &message) {
      if (this->pingData) {
        delete this->pingData;
        this->pingData = nullptr;
      }
      if (!message.payloadString().isEmpty())
        this->pingData = new PingData(message.payloadString());
    });
  }

  void loop() override {
    Actor<PROCESS, PAYLOAD, ROUTER>::loop();
    // 64 bytes from 172.217.12.142: icmp_seq=0 ttl=116 time=87.243 ms
    if (this->pingData) {
      this->pingData->counter++;
      char *message = new char[100];
      if (this->xping.ping(this->pingData->host.c_str(), 1)) {
        this->pingData->success++;
        // this->pingData->totalTime += ::Ping.averageTime();
        sprintf(message, "64 bytes from %s: icmp_seq=%i time=%.3f ms\n",
                this->pingData->ip.c_str(), this->pingData->counter,
                this->xping.averageTime());
        this->publish(fWIFI::idFromIP("log", "INFO"), String(message));
      } else {
        sprintf(message,
                "Request timeout for icmp_seq %i failure_rate=%.2f%%\n",
                this->pingData->counter, this->pingData->failureRate());
        this->publish(fWIFI::idFromIP("log", "ERROR"), String(message));
      }
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
    [[nodiscard]]  float failureRate() const {
      return (((float)(counter - success)) / ((float)counter)) * 100.0f;
    }
    // const float successRate() const { return success / counter; }
    // const float averageTime() const { return totalTime / success; }
  };
  ::PingClass xping = ::Ping;
  PingData *pingData = nullptr;
};
} // namespace fhatos::kernel

#endif