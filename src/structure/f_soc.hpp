#ifndef fhatos_f_soc_hpp
#define fhatos_f_soc_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <util/i2c.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)

namespace fhatos {

template <typename PROCESS = Fiber, typename ROUTER = LocalRouter<>>
class fSoC final : public Actor<PROCESS, ROUTER> {

protected:
  explicit fSoC(const ID &id = fWIFI::idFromIP("kernel", "soc"))
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
    PROCESS::setup();
    this->subscribe(this->id(), [this](const Message &message) {
      if (message.payload.toString() == "?out")
        this->updateBBS(message.target.query("out"));
    });

    this->subscribe(this->id().extend("i2c/+"), [this](const Message &message) {
      String pins = message.target.lastSegment();
      int i = pins.indexOf(",");
      uint8_t sda = pins.substring(0, i).toInt();
      uint8_t scl = pins.substring(i + 1).toInt();
    });
    ///////////////////////////// GPIO /////////////////////////////
    this->subscribe(this->id().extend("gpio/+"), [this](
                                                     const Message &message) {
      const int pin = message.target.lastSegment().toInt();
      const bool canOutput = digitalPinCanOutput(pin);
      LOG(canOutput ? INFO : ERROR, "Writing !g%s!! to digital pin !b%i!!\n",
          message.payload.toBool().value() ? "HIGH" : "LOW",
          message.target.lastSegment().toInt());
      if (canOutput)
        digitalWrite(pin, message.payload.toBool().value() ? HIGH : LOW);
    });
    /////////////////////////////  PWM  /////////////////////////////
    this->subscribe(this->id().extend("pwm/+"), [this](const Message &message) {
      const int pin = message.target.lastSegment().toInt();
      const bool canOutput = digitalPinHasPWM(pin);
      LOG(canOutput ? INFO : ERROR, "Writing !g%i!! to analaog pin !b%i!!\n",
          message.payload.toInt(), message.target.lastSegment().toInt());
      if (canOutput)
        analogWrite(pin, message.payload.toInt().value());
    });
  }

  virtual void loop() override {}

private:
  void updateBBS(const ID &queryId) {
    LOG(INFO, "Scanning I2C pins");
    String message = "\n!M!_" + this->id().toString() + "!!\n";
    for (Pair<i2c_pins, List<Pair<uint8_t, const char *>>> i2c :
         I2C::i2cFullScan()) {

      for (Pair<uint8_t, const char *> single : i2c.second) {
        char line[512];
        sprintf(line,
                FOS_TAB "!b\\_!!!ri2c/%i_%i!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                        "[!gaddr!!:!b0x%x!!][!gid!!:!b%x!!][!gdesc!!:!b%s!!]\n",
                i2c.first.first, i2c.first.second, single.first, single.first,
                single.second);
        message = message + line;
      }
    }
    this->publish(queryId, message, RETAIN_MESSAGE);
  }
};

} // namespace fhatos

#endif