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
#ifndef fhatos_pin_driver_hpp
#define fhatos_pin_driver_hpp

#include <fhatos.hpp>

namespace fhatos {
  class PinDriver {
  public:
    virtual ~PinDriver() = default;

    virtual bool is_digital_pin(uint8_t pin) = 0;

    virtual void digital_write(uint8_t pin, bool value) = 0;

    virtual bool digital_read(uint8_t pin) = 0;

    virtual bool is_analog_pin(uint8_t pin) = 0;

    virtual void analog_write(uint8_t pin, uint16_t value) = 0;

    virtual uint16_t analog_read(uint8_t pin) = 0;
  };


#ifdef ESP_ARCH
#define FOS_PWM_ANALOG_RESOLUTION 4095
  class ArduinoPinDriver : public PinDriver {
  protected:
    ArduinoPinDriver() : PinDriver() {}

  public:
    bool is_digital_pin(const uint8_t pin) override { return digitalPinIsValid(pin); } 
    // digitalPinCanOutput(pin)

    void digital_write(const uint8_t pin, const bool value) override {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, value ? 255 : 0);
    }

    bool digital_read(const uint8_t pin) override {
      //pinMode(pin, INPUT);
      return digitalRead(pin) > 0;
    }

    bool is_analog_pin(const uint8_t pin) override { return digitalPinHasPWM(pin) ; }

    void analog_write(const uint8_t pin, const uint16_t value) override { analogWrite(pin, value); }

    uint16_t analog_read(const uint8_t pin) override { return analogRead(pin); }

    static ptr<ArduinoPinDriver> singleton() {
      static ptr<ArduinoPinDriver> driver = ptr<ArduinoPinDriver>(new ArduinoPinDriver());
      return driver;
    
    }
  };
#elif defined(NATIVE)
  class NativePinDriver final : public PinDriver {
  protected:
    Pattern gpio_pattern_;
    Pattern pwm_pattern_;

  protected:
    NativePinDriver(const Pattern &gpio_pattern, const Pattern &pwm_pattern) : PinDriver(), gpio_pattern_(gpio_pattern),
                                                                               pwm_pattern_(pwm_pattern) {
    }

  public:
    static ptr<NativePinDriver> create(const Pattern &gpio_pattern, const Pattern &pwm_pattern) {
      const auto driver = ptr<NativePinDriver>(new NativePinDriver(gpio_pattern, pwm_pattern));
      return driver;
    }

    bool is_digital_pin(const uint8_t pin) override { return true; }

    void digital_write(const uint8_t pin, const bool value) override {
      router()->write(furi_p(this->gpio_pattern_.resolve(string("./") + to_string(pin))), dool(value));
    }

    bool digital_read(const uint8_t pin) override {
      const Bool_p b = router()->read(furi_p(this->gpio_pattern_.resolve(string("./") + to_string(pin))));
      return !b->is_noobj() && b->bool_value();
    }

    bool is_analog_pin(const uint8_t pin) override { return true; }

    void analog_write(const uint8_t pin, const uint16_t value) override {
      router()->write(furi_p(this->pwm_pattern_.resolve(string("./") + to_string(pin))), jnt(value));
    }

    uint16_t analog_read(const uint8_t pin) override {
      const Int_p i = router()->read(furi_p(this->gpio_pattern_.resolve(string("./") + to_string(pin))));
      return !i->is_noobj() && i->int_value();
    }
  };
#endif
} // namespace fhatos
#endif
