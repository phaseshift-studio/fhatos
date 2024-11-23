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
#ifndef fhatos_arduino_i2c_master_driver_hpp
#define fhatos_arduino_i2c_master_driver_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/parser.hpp>
#include <structure/router.hpp>

#include <model/driver/driver.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#endif

namespace fhatos {
  class ArduinoI2CDriver {
  public:
    static Obj_p load_remote(const ID &driver_value_id, const ID_p &driver_remote_id,
                             const ID &define_ns_prefix = ID("i2c")) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":setup"))
              ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
                ROUTER_WRITE(id_p(driver_remote_id->extend(":setup")), ObjHelper::make_lhs_args(lhs, {}), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":stop"))
              ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
                ROUTER_WRITE(id_p(driver_remote_id->extend(":stop")), ObjHelper::make_lhs_args(lhs, {}), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":tx"))
              ->type_args(x(0, "address"), x(1, "data size"))
              ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
                ROUTER_WRITE(id_p(driver_remote_id->extend(":read")), ObjHelper::make_lhs_args(lhs, args), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":write"))
              ->type_args(x(0, "data array", str("")))
              ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
                ROUTER_WRITE(id_p(driver_remote_id->extend(":write")), ObjHelper::make_lhs_args(lhs, args), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":read"))
              ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
                ROUTER_WRITE(id_p(driver_remote_id->extend(":read")), ObjHelper::make_lhs_args(lhs, {}), TRANSIENT);
                return noobj();
              })
              ->create(),
      });
      Type::singleton()->save_type(
          id_p("/lib/driver/i2c/arduino/master/furi"),
          rec({{vri(":create"),
                ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":create"))
                    ->type_args(x(0, "local_id", vri(driver_value_id)), x(1, "remote_id", vri(driver_remote_id)),
                                x(2, "ns_prefix", vri(define_ns_prefix)))
                    ->instance_f([inst_types](const Obj_p &, const InstArgs &args) {
                      const Rec_p record = rec();
                      for (const auto &i: *inst_types) {
                        record->rec_set(vri(i->inst_op()), i);
                      }
                      const Uri_p driver_id = args.at(0);
                      const Uri_p ns_prefix = args.at(2);
                      if (!ns_prefix->is_noobj())
                        Type::singleton()->save_type(id_p(ID(FOS_NAMESPACE_PREFIX_ID).extend(ns_prefix->uri_value())),
                                                     driver_id);
                      return record->at(id_p(args.at(0)->uri_value()));
                    })
                    ->create()}}));
      return noobj();
    }

    ///////////////////////////////////////////////////////
    /////////////// ARDUINO HARDWARE DRIVER ///////////////
    ///////////////////////////////////////////////////////
#ifdef ESP_ARCH
#define FOS_I2C_SDA_PIN 8
#define FOS_I2C_SCL_PIN 9
    /*
    void init(void);
      uint8_t read(void);
      uint8_t readLast(void);
      bool write(uint8_t data);
      bool start(uint8_t address, int32_t readcount);
      bool restart(uint8_t address, int32_t readcount);
      void stop(void);
     */

    /*
    endTransmission error codes:
    0: success.
1: data too long to fit in transmit buffer.
2: received NACK on transmit of address.
3: received NACK on transmit of data.
4: other error.
5: timeout
    */

  private:
    static uint8_t i2c_write(const string hex_address, const string hex_data, const bool end_tx = true) {
      const int address = StringHelper::hex_to_int(StringHelper::clip_0x(hex_address));
      const List<fbyte> data = StringHelper::hex_to_bytes(StringHelper::clip_0x(hex_data));
      Wire.beginTransmission(address);
      uint8_t size = Wire.write(data.data(), data.size());
      LOG(TRACE, "writing %s to %x/%i\n", StringHelper::bytes_to_hex(data), address, address);
      if (end_tx) {
        uint8_t result = Wire.endTransmission(true);
        if (result == 1)
          LOG(ERROR, "%i with 0x%s: !yoverflow transit buffer!!\n", hex_address, hex_data);
        else if (result == 2)
          LOG(ERROR, "%i with 0x%s: !rnack !ytransit!! address\n", hex_address, hex_data);
        else if (result == 3)
          LOG(ERROR, "%i with 0x%s: !rnack !ytransit!! data\n", hex_address, hex_data);
        else if (result == 4)
          LOG(ERROR, "%i with 0x%s: unknown error\n", hex_address, hex_data);
        else if (result == 5)
          LOG(ERROR, "%i with 0x%s: !ytimeout!!\n", hex_address, hex_data);
        if (result != 0)
          Wire.clearWriteError();
        return result;
      }
      return size == 0 ? 4 : 0;
    }

    static string i2c_read(const string hex_address, const uint8_t size, const bool end = true) {
      const int address = StringHelper::hex_to_int(StringHelper::clip_0x(hex_address));
      const uint8_t result_size = Wire.requestFrom(address, size, end);
      List<fbyte> data;
      while (Wire.available()) {
        data.push_back(Wire.read()); // Receive a byte & push it into the RxBuffer
      }
      if (data.size() < size) {
        LOG(ERROR, "only %i bytes of the requested %i received\n", result_size, size);
      }
      const string hex_string = StringHelper::prefix_0x(StringHelper::bytes_to_hex(data));
      return hex_string;
    }

  public:
    static uint8_t i2c_scan(uint8_t scl, uint8_t sda, uint8_t low_addr = 8, uint8_t high_addr = 120) {
      uint8_t count = 0;
      Wire.begin(sda, scl);
      const ptr<ProgressBar> pb = ProgressBar::start(printer().get(), high_addr - low_addr);
      for (byte i = low_addr; i < high_addr; i++) {
        Wire.beginTransmission(i);
        const uint8_t result = Wire.endTransmission();
        if (0 == result) {
          LOG(INFO, "  !ydevice at addr !b%i/0x%x!!\n", i, i);
          count++;
          // delay(1); // maybe unneeded?
        } else if (4 == result) {
          LOG(ERROR, "  !ydevice error at addr !b%i/0x%x!!\n", i, i);
        }
        pb->incr_count(to_string(i) + "addr");
      }
      pb->end("i2c addr range scan complete");
      Wire.end();
      LOG(NONE, "\n");
      LOG(INFO, "   !mi2c device(s) found at scl:%i/sda:%i: !g%i!!\n", scl, sda, count);
      delay(1000);
      return count;
    }
    static Obj_p load_local(const ID &driver_value_id, const ID_p &driver_remote_id,
                            const ID &define_ns_prefix = ID("i2c")) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":setup"))
              ->type_args(x(0, "scl"), x(1, "sda"))
              ->instance_f([](const Obj_p &, const InstArgs &args) {
                const uint8_t scl = args.at(0)->int_value();
                const uint8_t sda = args.at(1)->int_value();
                pinMode(scl, PULLUP);
                pinMode(sda, PULLUP);
                if (0 == i2c_scan(scl, sda))
                  LOG(WARN, "no i2c devices found on scl:%i/sda:%i\n", scl, sda);
                Wire.begin(sda, scl);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":stop"))
              ->instance_f([](const Obj_p &, const InstArgs &) {
                Wire.end();
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":write"))
              ->type_args(x(0, "address"), x(1, "data"), x(2, "end_tx", dool(true)))
              ->instance_f([](const Obj_p &, const InstArgs &args) {
                const uint8_t result = i2c_write(args.at(0)->uri_value().toString(), args.at(1)->uri_value().toString(),
                                                 args.at(2)->bool_value());
                return jnt(result);
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":read"))
              ->type_args(x(0, "address"), x(1, "size"), x(2, "end_tx", dool(true)))
              ->instance_f([](const Obj_p &, const InstArgs &args) {
                const string result =
                    i2c_read(args.at(0)->uri_value().toString(), args.at(1)->int_value(), args.at(2)->bool_value());
                return vri(result);
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":scan"))
              ->type_args(x(0, "scl", noobj()), x(1, "sda", noobj()), x(2, "low_addr", jnt(8)),
                          x(3, "high_addr", jnt(120)))
              ->instance_f([](const Obj_p &, const InstArgs &args) {
                LOG(INFO, "!g[!bi2c Configuration!g]!!\n");
                const uint8_t low_addr = args.at(2)->int_value();
                const uint8_t high_addr = args.at(3)->int_value();
                if (args.at(0)->is_noobj() || args.at(1)->is_noobj()) {
                  uint8_t lowPin = 0;
                  uint8_t highPin = 40;
                  for (uint8_t i = lowPin; i <= highPin; i++) {
                    for (uint8_t j = lowPin; j <= highPin; j++) {
                      FEED_WATCDOG();
                      if (i != j && i != 1 && j != 1 && i != 3 && j != 3) {
                        LOG(INFO, "  !yscanning all addrs on pins !bscl:!y%i!!:!bsda!y%i!!\n", i, j);
                        if (i2c_scan(i, j, low_addr, high_addr) > 0) {
                          LOG(INFO, "  !yi2c pins at !bscl:!y%i!!:!bsda:!y%i!!\n", i, j);
                        }
                      }
                    }
                  }
                  return jnt(0);
                } else {
                  return jnt(i2c_scan(args.at(0)->int_value(), args.at(1)->int_value(), args.at(2)->int_value(),
                                      args.at(3)->int_value()));
                }
              })
              ->create()});


      //////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////// ARDUINO DRIVER INSTALLATION ///////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
          id_p("/lib/driver/i2c/arduino/master/pin"),
          rec({{vri(":create"),
                ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":create"))
                    ->type_args(x(0, "local_in", vri(driver_value_id)), x(1, "remote_id", vri(driver_remote_id)),
                                x(2, "ns_prefix", vri(define_ns_prefix)))
                    ->instance_f([inst_types](const Obj_p &lhs, const InstArgs &args) {
                      const Rec_p record = rec();
                      for (const auto &i: *inst_types) {
                        record->rec_set(vri(i->inst_op()), i);
                      }
                      ROUTER_SUBSCRIBE(Subscription::create(args.at(0)->uri_value(),
                                                            args.at(1)->uri_value().extend(":setup/0"),
                                                            Obj::to_bcode(
                                                                [args](const Obj_p &, const InstArgs &args2) {
                                                                  const uint8_t scl = args2.at(0)->int_value();
                                                                  const uint8_t sda = args2.at(1)->int_value();
                                                                  Wire.begin(sda, scl);
                                                                  return noobj();
                                                                },
                                                                {x(0), x(1)})));
                      ROUTER_SUBSCRIBE(Subscription::create(args.at(0)->uri_value(),
                                                            args.at(1)->uri_value().extend(":stop/0"),
                                                            Obj::to_bcode([lhs, args](const Obj_p &) {
                                                              Wire.end();
                                                              return noobj();
                                                            })));
                      ROUTER_SUBSCRIBE(Subscription::create(
                          args.at(0)->uri_value(), args.at(1)->uri_value().extend(":write/0"),
                          Obj::to_bcode(
                              [lhs, args](const Obj_p &, const InstArgs &args2) {
                                const uint8_t result =
                                    i2c_write(args.at(0)->uri_value().toString(), args.at(1)->uri_value().toString(),
                                              args.at(2)->bool_value());
                                return jnt(result);
                              },
                              {x(0), x(1), x(2, dool(true))})));
                      ROUTER_SUBSCRIBE(Subscription::create(
                          args.at(0)->uri_value(), args.at(1)->uri_value().extend(":read/0"),
                          Obj::to_bcode(
                              [lhs, args](const Obj_p &, const InstArgs &args2) {
                                const string result =
                                    i2c_read(args.at(0)->uri_value().toString(), args.at(1)->int_value());
                                const Uri_p r = vri(result);
                                ROUTER_WRITE(id_p(args.at(1)->uri_value().extend(":read/1")), r, RETAIN);
                                return r;
                              },
                              {x(0), x(1)})));
                      const Uri_p driver_id = args.at(0);
                      const Uri_p ns_prefix = args.at(2);
                      if (!ns_prefix->is_noobj())
                        Type::singleton()->save_type(id_p(ID(FOS_NAMESPACE_PREFIX_ID).extend(ns_prefix->uri_value())),
                                                     driver_id);
                      return record->at(id_p(driver_id->uri_value()));
                    })
                    ->create()}}));
      return noobj();
    };
#endif
  };
} // namespace fhatos
#endif
