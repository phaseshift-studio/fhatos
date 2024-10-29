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
#ifndef fhatos_driver_hpp
#define fhatos_driver_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/stype/heap.hpp>
#include <model/driver/digital_pin_diver.hpp>
#include <model/driver/analog_pin_driver.hpp>
#include <model/driver/i2c_driver.hpp>

namespace fhatos {
  class Driver : public Heap {
    explicit Driver(const Pattern &pattern) : Heap(pattern) {
    }

    void setup() override {
      Heap::setup();
      this->write_raw_pairs(id_p(this->pattern()->resolve(string("./protocol/pwm/0"))),
                            vri(fURIAnalogPinDriver::static_id()), true);
      this->write_raw_pairs(id_p(this->pattern()->resolve(string("./protocol/gpio/0"))),
                            vri(fURIDigitalPinDriver::static_id()), true);
#ifdef ESP_ARCH
      this->write_raw_pairs(id_p(this->pattern()->resolve(string("./protocol/pwm/1"))), vri(ArduinoAnalogPinDriver::static_id()), true);
      this->write_raw_pairs(id_p(this->pattern()->resolve(string("./protocol/gpio/1"))), vri(ArduinoDigitalPinDriver::static_id()), true);
      this->write_raw_pairs(id_p(this->pattern()->resolve(string("./protocol/i2c/0"))), vri(ArduinoI2CDriver::static_id()), true);
#endif
    }

  public:
    static ptr<Driver> singleton(const Pattern &pattern) {
      static auto sysp = ptr<Driver>(new Driver(pattern));
      return sysp;
    }
  };
}

#endif
