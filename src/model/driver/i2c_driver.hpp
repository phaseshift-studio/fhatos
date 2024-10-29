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
  class I2CDriver : BaseDriver {
  public:
    explicit I2CDriver(const ID &id) : BaseDriver(id, PROTOCOL::I2C) {
    }

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
    ArduinoI2CDriver() : I2CDriver(*ArduinoI2CDriver::static_id()) {}

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

    static ID_p static_id() {
      return id_p("/driver/arduino/i2c");
    }
    static ptr<ArduinoI2CDriver> singleton() {
      static ptr<ArduinoI2CDriver> driver = ptr<ArduinoI2CDriver>(new ArduinoI2CDriver());
      return driver;
    }
  };
#endif

  class fURII2CDriver : public I2CDriver {
  protected:
    const ID_p write_id_;
    const ID_p read_id_;
    const Pattern_p address_prefix_;
    const Uri_p write_address;

    explicit fURII2CDriver(const ID &write_id, const ID &read_id,
                  const Pattern &address_prefix) : I2CDriver(*fURII2CDriver::static_id()),
                                                   write_id_(id_p(write_id)), read_id_(id_p(read_id)),
                                                   address_prefix_(p_p(address_prefix)) {
    }

  public:
    void begin() override { router()->write(this->write_id_, vri(":begin")); }
    void end() override { router()->write(this->write_id_, vri(":end")); }

    int requestFrom(const int address, const int quantity) override {
      return 1;
      // router()->write(this->read_id_,)
    }

    void beginTransmission(const int address) override {
      router()->write(this->write_id_,
                      lst({vri(":begin"), vri(this->address_prefix_->resolve(string("./") + to_string(address)))}));
    }

    int endTransmission() override {
      router()->write(this->write_id_,
                      lst({vri(":end"), noobj()}));
      return router()->read(this->read_id_)->int_value();
    }

    size_t write(const uint8_t data) override {
      router()->write(this->write_id_, jnt(data));
      return 1;
    }

    int available() override {
      return 1;
      //  return Wire.available();
    }

    int read() override {
      return router()->read(this->read_id_)->int_value();
    }

    void setClock(uint32_t frequency) override {
      //Wire.setClock(frequency);
    }

    void onReceive(void (*function)(int)) override {
      //    Wire.onReceive(function);
    }

    void onRequest(void (*function)(void)) override {
      //  Wire.onRequest(function);
    }

    static ID_p static_id() {
      return id_p("/driver/furi/i2c");
    }

    static ptr<fURII2CDriver> create() {
      static ptr<fURII2CDriver> driver = ptr<fURII2CDriver>(new fURII2CDriver("a", "b", "c"));
      return driver;
    }
  };
} // namespace fhatos
#endif
