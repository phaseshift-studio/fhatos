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
#include <model/soc/esp/pin.hpp>
#include <structure/stype/external.hpp>

#define FOS_PWM_ANALOG_RESOLUTION 4095

namespace fhatos {

  class PWM : public Pin {

  protected:
    Map<ID_p, BCode_p, furi_p_less> interrupts_;
    explicit PWM(const Pattern &pattern = "/soc/pwm/#") :
        Pin(
            pattern,
            [](const uint8_t pin) -> Int_p {
              // pinMode(pin, INPUT);
              return digitalPinHasPWM(pin) ? jnt(::map(analogRead(pin), 0, FOS_PWM_ANALOG_RESOLUTION, 0, 255))
                                           : noobj();
            },
            [](const uint8_t pin, const int value) -> void {
              if (digitalPinHasPWM(pin))
                analogWrite(pin, value);
            }) {}

  public:
    static ptr<PWM> singleton(const Pattern &pattern = "/soc/pwm/#") {
      static ptr<PWM> pwm = ptr<PWM>(new PWM(pattern));
      return pwm;
    }
  };
} // namespace fhatos
#endif