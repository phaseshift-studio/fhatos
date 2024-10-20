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
#ifndef fhatos_gpio_hpp
#define fhatos_gpio_hpp

#include <fhatos.hpp>
#include <language/processor/processor.hpp>
#include <model/soc/esp/pin.hpp>

namespace fhatos {

  class GPIO : public Pin {
  protected:
    explicit GPIO(const Pattern &pattern = "/soc/gpio/#") :
        Pin(
            pattern,
            [](const uint8_t pin) -> Int_p {
              // pinMode(pin, INPUT);
              return digitalPinIsValid(pin) ? jnt(digitalRead(pin)) : noobj();
            },
            [](const uint8_t pin, const Int_p& value) {
              if (digitalPinIsValid(pin) && digitalPinCanOutput(pin))
                digitalWrite(pin, value->int_value());
            }) {}

  public:
    static ptr<GPIO> singleton(const Pattern &pattern = "/soc/gpio/#") {
      static ptr<GPIO> gpio = ptr<GPIO>(new GPIO(pattern));
      return gpio;
    }
  };
} // namespace fhatos
#endif