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
#ifndef fhatos_arduino_i2c_hpp
#define fhatos_arduino_i2c_hpp
#ifndef NATIVEd

#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../../lang/type.hpp"
#include "../../../structure/router.hpp"
#include "../../../util/obj_helper.hpp"
#include STR(../../../process/ptype/HARDWARE/scheduler.hpp)

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif
// #ifdef NATIVE
// #include <wiringPi.h>
// #endif

#define FOS_I2C_ADDR_STR "0x%x/%i"
#define FOS_I2C_ADDR(a) a, a

namespace fhatos {
  static ID_p I2C_FURI = id_p("/fos/i2c");

  class ArduinoI2C final : public Rec {

  public:
    static std::pair<const char *, const char *> i2c_device_description(const int addr) {
      switch(addr) {
        case 0x20: // 32
          return {"pcf8575", "gpio exapnder"};
        case 0x38: // 56
          return {"aht10", "temp/humidity"};
        case 0x39: // 57
          return {"aht10", "temp/humidity"};
        case 0x3c: // 60
          return {"oled", "screen"};
        case 0x40: // 64
          return {"pca9685", "pwm expander"};
        case 0x70: // 112
          return {"tca9548a", "i2c expander"};
        case 0x77: // 119
          return {"bme680", "environment sensor"};
        default:
          return {"unknown", "no description"};
      }
    }

    explicit ArduinoI2C(const int sda_pin, const int scl_pin, const int freq, const ID_p &value_id = nullptr) :
      Rec(rmap({{"sda", jnt(sda_pin)},
                {"scl", jnt(scl_pin)},
                {"freq", jnt(freq)}}),
          OType::REC, I2C_FURI, value_id) {
    }

    static inline void *import(const ID &lib_id = "/io/lib/i2c") {
      Typer::singleton()->save_type(I2C_FURI, Obj::to_rec({
                                        {"sda", Obj::to_type(INT_FURI)},
                                        {"scl", Obj::to_type(INT_FURI)}}));
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, I2C_FURI, {1, 1})
          ->inst_args(rec({{"sda", Obj::to_type(INT_FURI)},
                           {"scl", Obj::to_type(INT_FURI)},
                           {"freq", jnt(50000)},
                           {"id", Obj::to_noobj()}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return make_shared<Obj>(
                ArduinoI2C(
                    args->arg("sda")->int_value(),
                    rgs->arg("scl")->int_value(),
                    args->arg("freq")->int_value(),
                    args->arg("id")->is_noobj() ? nullptr : id_p(args->arg("id")->uri_value())));
          })
          ->save();
      //////////////////////////////////////////////////////////////////////////////////
      InstBuilder::build(ID(lib_id.extend(":find")))
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
                if(i != j && i != 1 && j != 1 && i != 3 && j != 3 /*&&
                            Helper::gpioToD(i) != 111 && Helper::gpioToD(j) != 111*/) {
                  const ArduinoI2C temp_i2c = ArduinoI2C(i, j);
                  LOG(INFO, FOS_TAB_1 "!ychecking existence of %s\n", temp_i2c.toString().c_str());
                  int found = temp_i2c.rec_get(":scan")->apply(Obj::to_noobj())->int_value();
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
          ->save();
      InstBuilder::build(I2C_FURI->add_component("scan"))
          ->domain_range(OBJ_FURI, {0, 1}, LST_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Lst_p result_set = Obj::to_lst();
            Rec_p i2c_rec = lhs;
            try {
              if(!Wire.begin(i2c_rec->rec_get("sda")->int_value(),
                             i2c_rec->rec_get("scl")->int_value())) {
                LOG_OBJ(ERROR, i2c_rec, "!runable to communicate!! with %s!!\n",
                        i2c_rec->toString().c_str());
                return result_set;
              }
              for(int i = 8; i < 120; i++) {
                Scheduler::singleton()->feed_local_watchdog();
                Wire.beginTransmission(i);
                const int result = Wire.endTransmission();
                if(0 == result || 4 == result) {
                  const auto desc = i2c_device_description(i);
                  result_set->lst_add(Obj::to_rec({
                      {"hex_addr", vri(StringHelper::int_to_hex(i).insert(0, "0x"))},
                      {"int_addr", jnt(i)},
                      {"model", Obj::to_str(desc.first)},
                      {"type", Obj::to_str(desc.second)},
                      {"status", Obj::to_str(0 == result ? "ok" : "error")}}));
                }
              }
            } catch(const std::exception &e) {
              Wire.end();
              throw fError("i2c %s bus error: %s", i2c_rec->toString().c_str(), e.what());
            }
            Wire.end();
            return result_set;
          })
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
#endif
