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
#ifndef fhatos_i2c_driver_hpp
#define fhatos_i2c_driver_hpp

#include <fhatos.hpp>
#include <structure/router.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#endif

namespace fhatos {
  class I2CDriver {
  public:
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual int requestFrom(int address, int quantity) = 0;
    virtual void beginTransmission(int address) = 0;
    virtual int endTransmission() = 0;
    virtual size_t write(uint8_t data) = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual void setClock(uint32_t frequency) = 0;
    virtual void onReceive(void (*function)(int)) = 0;
    virtual void onRequest(void (*function)(void)) = 0;
  };

#ifdef ESP_ARCH

  class ArduinoI2CDriver : public I2CDriver {
  protected:
    ArduinoI2CDriver() : I2CDriver() {}

  public:
    void begin() override { Wire.begin(); }
    void end() override { Wire.end(); }
    int requestFrom(const int address, const int quantity) override { return Wire.requestFrom(address, quantity); }
    void beginTransmission(int address) override { Wire.beginTransmission(address); }
    int endTransmission() override { return Wire.endTransmission(); }
    size_t write(const uint8_t data) override { return Wire.write(data); }
    int available() override { return Wire.available(); }
    int read() override { return Wire.read(); }
    void setClock(uint32_t frequency) override { Wire.setClock(frequency); }
    void onReceive(void (*function)(int)) override { Wire.onReceive(function); }
    void onRequest(void (*function)(void)) override { Wire.onRequest(function); }

    static ptr<ArduinoI2CDriver> singleton() {
      static ptr<ArduinoI2CDriver> driver = ptr<ArduinoI2CDriver>(new ArduinoI2CDriver());
      return driver;
    }
  };
#endif

} // namespace fhatos
#endif
