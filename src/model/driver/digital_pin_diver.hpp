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

#pragma once
#ifndef fhatos_digital_pin_driver_hpp
#define fhatos_digital_pin_driver_hpp

#include <structure/router.hpp>
#include <model/driver/base_driver.hpp>

namespace fhatos {
  class DigitalPinDriver : BaseDriver {
  public:
    explicit DigitalPinDriver(const ID& id) : BaseDriver(id, PROTOCOL::GPIO) {
    }

    ~DigitalPinDriver() override = default;

    // virtual void pinMode();

    virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;

    virtual int digitalRead(uint8_t pin) = 0;
  };


#ifdef ESP_ARCH

  class ArduinoDigitalPinDriver : public DigitalPinDriver {
  protected:
   explicit ArduinoDigitalPinDriver() : DigitalPinDriver(*ArduinoDigitalPinDriver::static_id()) {}

  public:
    static ID_p static_id()  {
      return id_p("/driver/arduino/digital");
    }
    void digitalWrite(const uint8_t pin, const uint8_t value) override {
      if (digitalPinCanOutput(pin)) {
        ::pinMode(pin, OUTPUT);
        ::digitalWrite(pin, value);
      }
    }

    int digitalRead(const uint8_t pin) override {
      // pinMode(pin, INPUT);
      return ::digitalRead(pin);
    }

    static ptr<ArduinoDigitalPinDriver> singleton() {
      static ptr<ArduinoDigitalPinDriver> driver = ptr<ArduinoDigitalPinDriver>(new ArduinoDigitalPinDriver());
      return driver;
    }
  };
#endif

  class fURIDigitalPinDriver final : public DigitalPinDriver {
  protected:
    Pattern read_digital_pattern_;
    Pattern write_digital_pattern_;

    explicit fURIDigitalPinDriver(const Pattern &read_digital_pattern,
                         const Pattern &write_digital_pattern) : DigitalPinDriver(*fURIDigitalPinDriver::static_id()),
                                                                 read_digital_pattern_(read_digital_pattern),
                                                                 write_digital_pattern_(write_digital_pattern) {
    }

  public:
    static ID_p static_id()  {
      return id_p("/driver/furi/digital");
    }
    static ptr<fURIDigitalPinDriver> create(const Pattern &read_digital_pattern, const Pattern &write_digital_pattern) {
      const auto driver =
          ptr<fURIDigitalPinDriver>(new fURIDigitalPinDriver(read_digital_pattern, write_digital_pattern));
      return driver;
    }

    void digitalWrite(const uint8_t pin, const uint8_t value) override {
      router()->write(furi_p(this->write_digital_pattern_.resolve(string("./") + to_string(pin))), jnt(value));
    }

    int digitalRead(const uint8_t pin) override {
      return router()->read(furi_p(this->read_digital_pattern_.resolve(string("./") + to_string(pin))))->int_value();
    }
  };
} // namespace fhatos
#endif
