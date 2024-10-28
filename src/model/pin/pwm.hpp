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
#ifndef fhatos_pwm_hpp
#define fhatos_pwm_hpp

#include <fhatos.hpp>
#include <model/pin/pin.hpp>
#include <model/driver/analog_pin_driver.hpp>

#define FOS_PWM_ANALOG_RESOLUTION 4095

namespace fhatos {
  template<typename ANALOG_PIN_DRIVER>
  class PWM : public Pin<ANALOG_PIN_DRIVER> {
    static_assert(std::is_base_of_v<AnalogPinDriver, ANALOG_PIN_DRIVER>,
                  "template must reference an analog pin driver");

  protected:
    explicit PWM(const Pattern &pattern, const ptr<ANALOG_PIN_DRIVER> driver) : Pin<ANALOG_PIN_DRIVER>(
      pattern,
      [this](const uint8_t pin) -> Int_p {
        return jnt(this->driver_->analogRead(pin));
      },
      [this](const uint8_t pin, const Int_p &value) -> void {
        this->driver_->analogWrite(pin, value->int_value());
      },
      driver) {
    }

  public:
    static ptr<PWM> create(const Pattern &pattern, const ptr<ANALOG_PIN_DRIVER> &driver) {
      auto pwm = ptr<PWM<ANALOG_PIN_DRIVER>>(new PWM<ANALOG_PIN_DRIVER>(pattern, driver));
      return pwm;
    }
  };
} // namespace fhatos
#endif
