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

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../gpio/gpio.hpp"
#include "../../../model.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#elif defined(RASPBERRYPI)
#include <wiringPi.h>
#elif defined(NATIVE)
#include "ext/Wire.h"
#endif

#define FOS_I2C_ADDR_STR "0x%x/%i"
#define FOS_I2C_ADDR(a) a, a
#ifndef GPIO_PIN_COUNT
#define GPIO_PIN_COUNT 128
#endif

namespace fhatos {
  static ID_p I2C_FURI = id_p("/fos/io/i2c");
  static int bus_num = 0;

  class I2C final : public Model<I2C> {
  protected:
   // TwoWire i2c_wire;

  public:
   //explicit I2C(TwoWire wire): i2c_wire(Wire) {
    //}
    explicit I2C() = default;

    static ptr<I2C> create_state(const Obj_p &i2c) {
      const uint8_t sda = i2c->rec_get("sda")->int_value();
      const uint8_t scl = i2c->rec_get("scl")->int_value();
      if(!Wire.begin(sda, scl)) {
        throw fError::create(i2c->toString(),
                             "!runable to communicate!! with i2c [sda:%i,scl:%i] hardware", sda, scl);
      }
      Wire.setClock(i2c->rec_get("freq")->int_value());
      const auto i2c_state = make_shared<I2C>();
      return i2c_state;
    }

    static Obj_p stop_inst(const Obj_p &i2c, const InstArgs &) {
      const ptr<I2C> i2c_state = I2C::get_state(i2c);
    Wire.end();
      LOG_WRITE(INFO, i2c.get(), L("!ywire communication!! stopped\n"));
      return Obj::to_noobj();
    }

    static Obj_p scan_inst(const Obj_p &i2c, const InstArgs &) {
      const ptr<I2C> i2c_state = I2C::get_state(i2c);
      const Lst_p result_set = Obj::to_lst();
      try {
        for(int i = 8; i < 120; i++) {
          FEED_WATCHDOG();
        Wire.beginTransmission(i);
          const int result =Wire.endTransmission();
          if(0 == result || 4 == result) {
            const auto [model, type] = i2c_device_description(i);
            result_set->lst_add(Obj::to_rec({
                {"hex_addr", vri(StringHelper::int_to_hex(i).insert(0, "0x"))},
                {"int_addr", jnt(i)},
                {"model", Obj::to_str(model)},
                {"type", Obj::to_str(type)},
                {"status", Obj::to_str(0 == result ? "ok" : "error")}}));
          }
        }
      } catch(const std::exception &e) {
      Wire.end();
        throw fError("i2c %s bus error: %s", i2c->toString().c_str(), e.what());
      }
      return result_set;
    }

    static void *import() {
      Typer::singleton()->save_type(*I2C_FURI, Obj::to_rec({
                                        {"sda", Obj::to_type(GPIO_FURI)},
                                        {"scl", Obj::to_type(GPIO_FURI)},
                                        {"freq", Obj::to_type(INT_FURI)}}));
      InstBuilder::build(I2C_FURI->add_component("stop"))
          ->domain_range(I2C_FURI, {1, 1}, NOOBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &i2c, const InstArgs &args) {
            return I2C::stop_inst(i2c, args);
          })
          ->save();
      InstBuilder::build(I2C_FURI->add_component("scan"))
          ->domain_range(I2C_FURI, {1, 1}, LST_FURI, {1, 1})
          ->inst_f([](const Obj_p &i2c, const InstArgs &args) {
            return I2C::scan_inst(i2c, args);
          })
          ->save();
      //////////////////////////////////////////////////////////////////////////////////
      /*InstBuilder::build(ID(I2C_FURI->extend(":find")))
          ->domain_range(OBJ_FURI, {0, 1}, LST_FURI, {1, 1})
          ->inst_args(rec({{"low_pin", jnt(0)}, {"high_pin", jnt(GPIO_PIN_COUNT)}, {"halt_on_find", dool(true)}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            int low_pin = args->arg("low_pin")->int_value();
            int high_pin = args->arg("high_pin")->int_value();
            bool halt_on_find = args->arg("halt_on_find")->bool_value();
            const Lst_p i2c_pins = Obj::to_lst();
            for(int i = low_pin; i <= high_pin; i++) {
              for(int j = low_pin; j <= high_pin; j++) {
                Scheduler::singleton()->feed_local_watchdog();
                if(i != j && i != 1 && j != 1 && i != 3 && j != 3 &&
                            Helper::gpioToD(i) != 111 && Helper::gpioToD(j) != 111) {
                  // const ArduinoI2C temp_i2c = ArduinoI2C(i, j);
                  // LOG(INFO, FOS_TAB_1 "!ychecking existence of %s\n", temp_i2c.toString().c_str());
                  /*int found = temp_i2c.rec_get(":scan")->apply(Obj::to_noobj())->int_value();
                  if(found > 0) {
                    LOG(INFO, "i2c pins located %i/%i", i, j);
                    i2c_pins->lst_add(Obj::to_rec({{"sda", jnt(i)}, {"scl", jnt(j)}}));
                    if(halt_on_find)
                      return i2c_pins;
                  }
                }
              }
            }
            return i2c_pins;
          })
          ->save();*/
      return nullptr;
    }

    static std::pair<const char *, const char *> i2c_device_description(const int addr) {
      switch(addr) {
        case 0x20: // 32
          return {"pcf8575", "gpio exapander"};
        case 0x38: // 56
          return {"aht10", "temp/humidity"};
        case 0x39: // 57
          return {"aht10", "temp/humidity"};
        case 0x3c: // 60
          return {"oled", "screen"};
        case 0x40: // 64
          return {"pca9685", "pwm expander"};
        case 0x5a: //90
          return {"mlx90614", "infrared thermometer"};
        case 0x70: // 112
          return {"tca9548a", "i2c expander"};
        case 0x77: // 119
          return {"bme680", "environment sensor"};
        default:
          return {"unknown", "no description"};
      }
    }

  };
} // namespace fhatos
#endif
