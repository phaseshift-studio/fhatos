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
#ifndef fhatos_base_driver_hpp
#define fhatos_base_driver_hpp

#include <language/obj.hpp>

namespace fhatos {
  enum class PROTOCOL { PWM, GPIO, I2C, SPI, MQTT };

  static const auto ProtocolTypes =
      Enums<PROTOCOL>({{PROTOCOL::GPIO, "gpio"}, {PROTOCOL::PWM, "pwm"}, {PROTOCOL::I2C, "i2c"}, {PROTOCOL::SPI, "spi"},
        {PROTOCOL::MQTT, "mqtt"}});

  static const ID_p DRIVER_TYPE = id_p(FOS_TYPE_PREFIX "/rec/driver/");
  static const ID_p PROTOCOL_TYPE_PREFIX = id_p(FOS_TYPE_PREFIX "/uri/protocol/");
  static const ID_p PROTOCOL_ID_PREFIX = id_p(FOS_TYPE_PREFIX "/protocol/");

  class BaseDriver : IDed {
  protected:
    const PROTOCOL protocol_;

    explicit BaseDriver(const ID &id, const PROTOCOL protocol) : IDed(id_p(id)), protocol_(protocol) {
    }

  public:
    PROTOCOL protocol() const {
      return this->protocol_;
    }
  };
}
#endif
