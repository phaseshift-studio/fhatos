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
#ifndef fhatos_i2c_hpp
#define fhatos_i2c_hpp

#include <fhatos.hpp>
#include <model/pin/pin.hpp>
#include <model/driver/i2c_driver.hpp>

namespace fhatos {
  template<typename I2C_DRIVER>
  class I2C : public Pin<I2C_DRIVER> {
    static_assert(std::is_base_of_v<I2CDriver, I2C_DRIVER>,
                  "template must reference a i2c driver");

  protected:
    explicit I2C(const Pattern &pattern, const ptr<I2C_DRIVER> &driver) : Pin<I2C_DRIVER>(
      pattern,
      [this](const uint8_t pin) -> Int_p {
        return noobj();
        //return jnt(this->driver_->digitalRead(pin));
      },
      [this](const uint8_t pin, const Int_p &value) {
        //this->driver_->digitalWrite(pin, value->int_value());
      },
      driver) {
    }

  public:
    static ptr<I2C> create(const Pattern &pattern,
                           const ptr<I2C_DRIVER> driver) {
      auto gpio = ptr<I2C>(new I2C(pattern, driver));
      return gpio;
    }
  };
} // namespace fhatos
#endif
