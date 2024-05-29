/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_f_soc_hpp
#define fhatos_f_soc_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <util/i2c.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fSoC final : public Actor<PROCESS, ROUTER> {
  protected:
    explicit fSoC(const ID &id = FOS_DEFAULT_ROUTER::mintID("soc"))
      : Actor<PROCESS, ROUTER>(id) {
    }

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
        if (message.payload->toString() == "?out")
          this->updateBBS(message.target.query("out"));
      });

      this->subscribe(this->id().extend("i2c/+"), [this](const Message &message) {
        string pins = message.target.lastSegment();
        int i = pins.find(',');
        uint8_t sda = stoi(pins.substr(0, i));
        uint8_t scl = stoi(pins.substr(i + 1));
      });
      ///////////////////////////// GPIO /////////////////////////////
      this->subscribe(this->id().extend("gpio/+"), [this](
                    const Message &message) {
                        const int pin = stoi(message.target.lastSegment());
                        const bool canOutput = digitalPinCanOutput(pin);
                        LOG(canOutput ? INFO : ERROR, "Writing !g%s!! to digital pin !b%i!!\n",
                            message.payload->toBool().value() ? "HIGH" : "LOW",
                            stoi(message.target.lastSegment()));
                        if (canOutput)
                          digitalWrite(pin, message.payload->toBool().value() ? HIGH : LOW);
                      });
      /////////////////////////////  PWM  /////////////////////////////
      this->subscribe(this->id().extend("pwm/+"), [this](const Message &message) {
        const int pin = stoi(message.target.lastSegment());
        const bool canOutput = digitalPinHasPWM(pin);
        LOG(canOutput ? INFO : ERROR, "Writing !g%i!! to analaog pin !b%i!!\n",
            message.payload->toInt(), stoi(message.target.lastSegment()));
        if (canOutput)
          analogWrite(pin, message.payload->toInt().value());
      });
    }

    virtual void loop() override {
    }

  private:
    void updateBBS(const ID &queryId) {
      LOG(INFO, "Scanning I2C pins");
      string message = string("\n!M!_").append(this->id().toString()).append("!!\n");
      for (Pair<i2c_pins, List<Pair<uint8_t, const char *> > > i2c:
           I2C::i2cFullScan()) {
        for (Pair<uint8_t, const char *> single: i2c.second) {
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
