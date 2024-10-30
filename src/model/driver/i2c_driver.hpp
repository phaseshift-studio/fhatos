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
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/type.hpp>
#include <structure/router.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#endif

namespace fhatos {
  class I2CDriver : BaseDriver {
  protected:
    static ID_p inst_i2c_id(const string &opcode) { return id_p(INST_FURI->resolve(string("i2c:") + opcode)); }

  public:
    explicit I2CDriver(const ID &id) : BaseDriver(id, PROTOCOL::I2C) {}

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

    static ID_p static_id() { return id_p("/driver/arduino/i2c"); }
    static ptr<ArduinoI2CDriver> singleton() {
      static bool setup = false;
      static ptr<ArduinoI2CDriver> driver = ptr<ArduinoI2CDriver>(new ArduinoI2CDriver());
      if (!setup) {
        setup = true;
        driver->setup();
      }
      return driver;
    }

  protected:
    static void setup() {
      Type::singleton()->save_type(inst_i2c_id("begin"), Obj::to_inst(
                                                             "begin", {},
                                                             [](const InstArgs &args) {
                                                               return [args](const Obj_p &lhs) {
                                                                 ArduinoI2CDriver::singleton()->begin();
                                                                 return noobj();
                                                               };
                                                             },
                                                             IType::ONE_TO_ZERO));
      Type::singleton()->save_type(inst_i2c_id("end"), Obj::to_inst(
                                                           "end", {},
                                                           [](const InstArgs &args) {
                                                             return [args](const Obj_p &lhs) {
                                                               ArduinoI2CDriver::singleton()->end();
                                                               return noobj();
                                                             };
                                                           },
                                                           IType::ONE_TO_ZERO));
      Type::singleton()->save_type(
          inst_i2c_id("request_from"),
          Obj::to_inst(
              "request_from", {x(0), x(1)},
              [](const InstArgs &args) {
                return [args](const Obj_p &lhs) {
                  return jnt(ArduinoI2CDriver::singleton()->requestFrom(args.at(0)->apply(lhs)->int_value(),
                                                                        args.at(1)->apply(lhs)->int_value()));
                };
              },
              IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("begin_transmission"),
                                   Obj::to_inst(
                                       "begin_transmission", {x(0)},
                                       [](const InstArgs &args) {
                                         return [args](const Obj_p &lhs) {
                                           ArduinoI2CDriver::singleton()->beginTransmission(
                                               args.at(0)->apply(lhs)->int_value());
                                           return noobj();
                                         };
                                       },
                                       IType::ONE_TO_ZERO));
      Type::singleton()->save_type(inst_i2c_id("end_transmission"),
                                   Obj::to_inst(
                                       "end_transmission", {},
                                       [](const InstArgs &) {
                                         return [](const Obj_p &lhs) {
                                           return jnt(ArduinoI2CDriver::singleton()->endTransmission());
                                         };
                                       },
                                       IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("write"), Obj::to_inst(
                                                             "write", {x(0)},
                                                             [](const InstArgs &args) {
                                                               return [args](const Obj_p &lhs) {
                                                                 return jnt(ArduinoI2CDriver::singleton()->write(
                                                                     args.at(0)->apply(lhs)->int_value()));
                                                               };
                                                             },
                                                             IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("available"),
                                   Obj::to_inst(
                                       "available", {},
                                       [](const InstArgs &) {
                                         return [](const Obj_p &lhs) {
                                           return jnt(ArduinoI2CDriver::singleton()->available());
                                         };
                                       },
                                       IType::ONE_TO_ONE));

      Type::singleton()->save_type(inst_i2c_id("read"), Obj::to_inst(
                                                            "read", {},
                                                            [](const InstArgs &) {
                                                              return [](const Obj_p &lhs) {
                                                                return jnt(ArduinoI2CDriver::singleton()->read());
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("set_clock"),
                                   Obj::to_inst(
                                       "set_clock", {x(0)},
                                       [](const InstArgs &args) {
                                         return [args](const Obj_p &lhs) {
                                           ArduinoI2CDriver::singleton()->setClock(args.at(0)->apply(lhs)->int_value());
                                           return noobj();
                                         };
                                       },
                                       IType::ONE_TO_ZERO));
    }
  };
#endif

  class fURII2CDriver : public I2CDriver {
  protected:
    const ID_p write_id_;
    const ID_p read_id_;
    const Pattern_p address_prefix_;
    const Uri_p write_address;

    explicit fURII2CDriver(const ID &write_id, const ID &read_id, const Pattern &address_prefix) :
        I2CDriver(*fURII2CDriver::static_id()), write_id_(id_p(write_id)), read_id_(id_p(read_id)),
        address_prefix_(p_p(address_prefix)) {}

  public:
    void begin() override { router()->write(this->write_id_, vri(":begin")); }
    void end() override { router()->write(this->write_id_, vri(":end")); }

    int requestFrom(const int address, const int quantity) override {
      router()->write(this->write_id_,
                      Obj::to_inst("i2c:request_from", {jnt(address), jnt(quantity)}, noobj_func(), IType::ONE_TO_ONE));
    }

    void beginTransmission(const int address) override {
       router()->write(this->write_id_,
                      Obj::to_inst("i2c:begin_transmission", {jnt(address)}, noobj_func(), IType::ONE_TO_ZERO));
    }

    int endTransmission() override {
      router()->write(this->write_id_, lst({vri(":end"), noobj()}));
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

    int read() override { return router()->read(this->read_id_)->int_value(); }

    void setClock(uint32_t frequency) override {
      // Wire.setClock(frequency);
    }

    void onReceive(void (*function)(int)) override {
      //    Wire.onReceive(function);
    }

    void onRequest(void (*function)(void)) override {
      //  Wire.onRequest(function);
    }

    static ID_p static_id() { return id_p("/driver/furi/i2c"); }

    static ptr<fURII2CDriver> create() {
      static bool setup_ = false;
      if(!setup_) {
        setup_ = true;
        fURII2CDriver::setup();
      }
      static ptr<fURII2CDriver> driver = ptr<fURII2CDriver>(new fURII2CDriver("a", "b", "c"));
      return driver;
    }

    static void setup() {
      Type::singleton()->save_type(inst_i2c_id("begin"), Obj::to_inst(
                                                             "begin", {},
                                                             noobj_func(),
                                                             IType::ONE_TO_ZERO));
      Type::singleton()->save_type(inst_i2c_id("end"), Obj::to_inst(
                                                           "end", {},
                                                           noobj_func(),
                                                           IType::ONE_TO_ZERO));
      Type::singleton()->save_type(
          inst_i2c_id("request_from"),
          Obj::to_inst(
              "request_from", {x(0), x(1)},
               noobj_func(),
              IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("begin_transmission"),
                                   Obj::to_inst(
                                       "begin_transmission", {x(0)},
                                       noobj_func(),
                                       IType::ONE_TO_ZERO));
      Type::singleton()->save_type(inst_i2c_id("end_transmission"),
                                   Obj::to_inst(
                                       "end_transmission", {},
                                       noobj_func(),
                                       IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("write"), Obj::to_inst(
                                                             "write", {x(0)},
                                                              noobj_func(),
                                                             IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("available"),
                                   Obj::to_inst(
                                       "available", {},
                                        noobj_func(),
                                       IType::ONE_TO_ONE));

      Type::singleton()->save_type(inst_i2c_id("read"), Obj::to_inst(
                                                            "read", {},
                                                             noobj_func(),
                                                            IType::ONE_TO_ONE));
      Type::singleton()->save_type(inst_i2c_id("set_clock"),
                                   Obj::to_inst(
                                       "set_clock", {x(0)},
                                        noobj_func(),
                                       IType::ONE_TO_ZERO));
    }

  };
} // namespace fhatos
#endif
