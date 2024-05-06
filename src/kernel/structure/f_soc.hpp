#ifndef fhatos_kernel__f_soc_hpp
#define fhatos_kernel__f_soc_hpp

#include <fhatos.hpp>
#include <kernel/furi.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>

namespace fhatos::kernel {

template <typename PROCESS, typename ROUTER>
class fSoC : public Actor<PROCESS, ROUTER> {

protected:
  fSoC(const ID &id = fWIFI::idFromIP("kernel", "soc"))
      : Actor<PROCESS, ROUTER>(id) {}

public:
  static fSoC *singleton() {
    static fSoC singleton = fSoC();
    return &singleton;
  }

  // kernel@127.0.0.1/soc/gpio/
  // kernel@127.0.0.1/soc/pwm/
  // kernel@127.0.0.1/soc/i2c/
  // kernel@127.0.0.1/soc/spi/

  virtual void setup() override {
    ///////////////////////////// GPIO /////////////////////////////
    this->subscribe(this->id().extend("gpio/+"),
                    [this](const Message &message) {
                      if (message.target.lastSegment().equals("RST")) {
                        if (message.payload.toBool()) {
                          ESP.restart();
                        }
                      } else {
                        LOG(INFO, "Writing !g%s!! to digital pin !b%i!!\n",
                            message.payload.toBool() ? "HIGH" : "LOW",
                            message.target.lastSegment().toInt());
                        digitalWrite(message.target.lastSegment().toInt(),
                                     message.payload.toBool() ? HIGH : LOW);
                      }
                    });
    /////////////////////////////  PWM  /////////////////////////////
    this->subscribe(this->id().extend("pwm/+"), [this](const Message &message) {
      LOG(INFO, "Writing !g%i!! to analaog pin !b%i!!\n", message.payload.toInt(),
          message.target.lastSegment().toInt());
      analogWrite(message.target.lastSegment().toInt(), message.payload.toInt());
    });
  }
};

} // namespace fhatos::kernel

#endif