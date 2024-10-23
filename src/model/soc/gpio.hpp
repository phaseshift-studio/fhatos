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
#include <model/soc/pin.hpp>

namespace fhatos {
  template<typename PIN_DRIVER>
  class GPIO : public Pin<PIN_DRIVER> {
  protected:
    explicit GPIO(const Pattern &pattern = "/soc/gpio/#", const ptr<PIN_DRIVER> &driver = nullptr) : Pin<PIN_DRIVER>(
      pattern,
      [this](const uint8_t pin) -> Bool_p {
        return this->driver_->is_digital_pin(pin) ? dool(this->driver_->digital_read(pin)) : noobj();
      },
      [this](const uint8_t pin, const Bool_p &value) {
        if (this->driver_->is_digital_pin(pin))
          this->driver_->digital_write(pin, value->bool_value());
      },
      driver) {
    }

  public:
    static ptr<GPIO> create(const Pattern &pattern = "/soc/gpio/#",
                            const ptr<PIN_DRIVER> driver = nullptr) {
      auto gpio = ptr<GPIO<PIN_DRIVER>>(new GPIO<PIN_DRIVER>(pattern, driver));
      return gpio;
    }
  };
} // namespace fhatos
#endif
