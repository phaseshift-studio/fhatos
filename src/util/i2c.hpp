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

#ifndef fhatos_i2c_hpp
#define fhatos_i2c_hpp

#include <Wire.h>
#include <fhatos.hpp>

namespace fhatos {

using i2c_pins = Pair<uint8_t, uint8_t>;

class I2C {
  I2C() = delete;

public:
  static const char *i2cToDevice(const uint8_t addr) {
    switch (addr) {
    case 0x20: // 32
      return "pcf8575 (gpio exapnder)";
    case 0x3c: // 60
      return "oled (screen)";
    case 0x40: // 64
      return "pca9685 (pwm expander)";
    case 0x70: // 112
      return "tca9548a (i2c expander)";
    case 0x77: // 119
      return "bme680 (environment sensor)";
    default:
      return "unknown";
    }
  }

#define GPIO_PIN_COUNT 40
  static Map<i2c_pins, List<Pair<uint8_t, const char *>>>
  i2cFullScan(const uint8_t lowPin = 0,
              const uint8_t highPin = GPIO_PIN_COUNT) {
    Map<i2c_pins, List<Pair<uint8_t, const char *>>> results;
    for (uint8_t i = lowPin; i <= highPin; i++) {
      for (uint8_t j = lowPin; j <= highPin; j++) {
        if (i != j && i != 1 && j != 1 && i != 3 && j != 3) {
          List<Pair<uint8_t, const char *>> scan = i2cScan(i, j);
          if (!scan.empty()) {
            results.emplace(i2c_pins{i, j}, scan);
          }
        }
      }
    }
    return results;
  }

  static List<Pair<uint8_t, const char *>> i2cScan(const uint8_t sda,
                                                   const uint8_t scl) {
    uint8_t count = 0;
    List<Pair<uint8_t, const char *>> results;
    Wire.begin(sda, scl);
    for (fbyte i = 8; i < 120; i++) {
      Wire.beginTransmission(i);
      const uint8_t result = Wire.endTransmission();
      if (0 == result) {
        count++;
        LOG(INFO, "\tDevice [type:%s] at [i2c:" FOS_I2C_ADDR_STR "]\n",
            i2cToDevice(i), FOS_I2C_ADDR(i));
        results.push_back({i, i2cToDevice(i)});
        delay(10); // maybe unneeded?
      } else if (4 == result) {
        LOG(ERROR, "\tDevice error at [i2c:" FOS_I2C_ADDR_STR "]\n",
            FOS_I2C_ADDR(i));
      }
    }
    LOG(INFO, "\t\tTotal I2C device(s) found: %i\n", count);
    return results;
  }
};

} // namespace fhatos

#endif