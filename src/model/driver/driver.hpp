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
#include <language/obj.hpp>
#include <furi.hpp>

/*
#define LOG_DRIVER(logtype, driver, format, ...)                                                                       \
  LOG((logtype), (string("!G[!Y%s!G]!! ") + (format)).c_str(), (driver)->vid()->toString().c_str(), ##__VA_ARGS__)
*/
namespace fhatos {
  enum class PROTOCOL { PWM, GPIO, I2C, SPI, MQTT };

  static const auto ProtocolTypes = Enums<PROTOCOL>({{PROTOCOL::GPIO, "gpio"},
                                                     {PROTOCOL::PWM, "pwm"},
                                                     {PROTOCOL::I2C, "i2c"},
                                                     {PROTOCOL::SPI, "spi"},
                                                     {PROTOCOL::MQTT, "mqtt"}});
  static const auto DRIVER_INST_FURI = id_p(FOS_SCHEME "/inst/driver/");

  class fDriver {
    explicit fDriver() = default;
  };
} // namespace fhatos
#endif