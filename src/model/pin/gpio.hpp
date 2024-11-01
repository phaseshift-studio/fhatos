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
#include <model/pin/pin.hpp>
#include <model/driver/fdriver.hpp>

namespace fhatos {
  template<typename DIGITAL_PIN_DRIVER>
  class GPIO : public Pin<DIGITAL_PIN_DRIVER> {
    static_assert(std::is_base_of_v<fDriver, DIGITAL_PIN_DRIVER>,
                  "template must reference a digital pin driver");

  protected:
    explicit GPIO(const Pattern &pattern, const ptr<DIGITAL_PIN_DRIVER> &driver) :
      Pin<DIGITAL_PIN_DRIVER>(
          pattern,
          [this](const uint8_t pin) -> Int_p {
            return jnt(this->driver_->digital_read(pin));
          },
          [this](const uint8_t pin, const Int_p &value) {
            this->driver_->digital_write(pin, value->int_value());
          },
          driver) {
    }

  public:
    static ptr<GPIO> create(const Pattern &pattern,
                            const ptr<DIGITAL_PIN_DRIVER> driver) {
      auto gpio = ptr<GPIO>(new GPIO(pattern, driver));
      return gpio;
    }
  };
} // namespace fhatos
#endif