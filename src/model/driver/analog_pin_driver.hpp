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
#ifndef fhatos_analog_pin_driver_hpp
#define fhatos_analog_pin_driver_hpp

#include <structure/router.hpp>

namespace fhatos {
  class AnalogPinDriver {
  public:
    virtual ~AnalogPinDriver() = default;

    virtual void analogWrite(uint8_t pin, int value) = 0;

    virtual uint16_t analogRead(uint8_t pin) = 0;
  };

#ifdef ESP_ARCH
#define FOS_PWM_ANALOG_RESOLUTION 4095

  class ArduinoAnalogPinDriver : public AnalogPinDriver {
  protected:
    ArduinoAnalogPinDriver() : AnalogPinDriver() {}

  public:
    void analogWrite(const uint8_t pin, const int value) override {
      ::pinMode(pin, OUTPUT);
      ::analogWrite(pin, value);
    }

    uint16_t analogRead(const uint8_t pin) override {
      ::pinMode(pin, INPUT);
      return ::analogRead(pin);
    }

    static ptr<ArduinoAnalogPinDriver> singleton() {
      static ptr<ArduinoAnalogPinDriver> driver = ptr<ArduinoAnalogPinDriver>(new ArduinoAnalogPinDriver());
      return driver;
    }
  };
#endif
  class fURIAnalogPinDriver final : public AnalogPinDriver {
  protected:
    Pattern read_analog_pattern_;
    Pattern write_analog_pattern_;

  protected:
    fURIAnalogPinDriver(const Pattern &read_analog_pattern, const Pattern &write_analog_pattern) :
        AnalogPinDriver(), read_analog_pattern_(read_analog_pattern), write_analog_pattern_(write_analog_pattern) {}

  public:
    static ptr<fURIAnalogPinDriver> create(const Pattern &read_analog_pattern, const Pattern &write_analog_pattern) {
      const auto driver = ptr<fURIAnalogPinDriver>(new fURIAnalogPinDriver(read_analog_pattern, write_analog_pattern));
      return driver;
    }

    void analogWrite(const uint8_t pin, const int value) override {
      router()->write(furi_p(this->write_analog_pattern_.resolve(string("./") + to_string(pin))), jnt(value));
    }

    uint16_t analogRead(const uint8_t pin) override {
      return router()->read(furi_p(this->read_analog_pattern_.resolve(string("./") + to_string(pin))))->int_value();
    }
  };
} // namespace fhatos
#endif
