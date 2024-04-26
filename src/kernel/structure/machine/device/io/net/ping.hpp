#ifndef fhatos_kernel__ping_hpp
#define fhatos_kernel__ping_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <ESPping.h>

namespace fhatos::kernel {

class Ping : public Actor<Fiber, StringMessage, LocalRouter<StringMessage>> {
public:
  Ping(const ID &id = WIFI::idFromIP("ping"))
      : Actor<Fiber, StringMessage, LocalRouter<StringMessage>>(id) {}

  virtual void setup() override {
    Actor::setup();
    this->subscribe(this->id(), [this](const StringMessage &message) {
      this->currentPing = message.payloadString();
      this->counter = 0;
      this->success = 0;
      this->failure = 0;
      this->totalTime = 0;
    });
  }

  virtual void loop() override {
    Actor::loop();
    // 64 bytes from 172.217.12.142: icmp_seq=0 ttl=116 time=87.243 ms
    if (!this->currentPing.isEmpty()) {
      counter++;
      char* message = new char[100];
      if (::Ping.ping(this->currentPing.c_str(), 1)) {
        this->success++;
        this->totalTime += ::Ping.averageTime();
        sprintf(message, "64 bytes from %s: icmp_seq=%i time=%.3f ms\n",
                this->currentPing.c_str(), this->counter,
                this->totalTime / (float)this->success);
        this->publish(WIFI::idFromIP("log", "INFO"), String(message));
      } else {
        this->failure++;
        sprintf(message,
                "Request timeout for icmp_seq %i failure_rate=%.2f%%\n",
                this->counter, (float)this->failure / (float)this->counter);
        this->publish(WIFI::idFromIP("log", "ERROR"), String(message));
      }
      delete message;
    }
    delay(1000);
  }

protected:
  String currentPing;
  uint counter = 0;
  uint success = 0;
  uint failure = 0;
  float totalTime = 0;
};
} // namespace fhatos::kernel

#endif