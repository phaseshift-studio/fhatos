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
#ifndef fhatos_arduino_i2c_driver_hpp
#define fhatos_arduino_i2c_driver_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/parser.hpp>
#include <structure/router.hpp>
#include <language/type.hpp>

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
          InstBuilder::build(driver_value_id.extend(":begin"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":begin")),
                         ObjHelper::make_lhs_args(lhs, {}),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":end"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":end")),
                         ObjHelper::make_lhs_args(lhs, {}),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":request_from"))
          ->type_args(x(0, "address"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":request_from")),
                         ObjHelper::make_lhs_args(lhs, args),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":begin_transmission"))
          ->type_args(x(0, "address"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":request_from")),
                         ObjHelper::make_lhs_args(lhs, args),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":end_transmission"))
          ->type_args(x(0, "stop", dool(true)))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":end_transmission")),
                         ObjHelper::make_lhs_args(lhs, args),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":write"))
          ->type_args(x(0, "data array", str("")))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":write")),
                         ObjHelper::make_lhs_args(lhs, args),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":available"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":available")),
                         ObjHelper::make_lhs_args(lhs, {}),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":read"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":read")),
                         ObjHelper::make_lhs_args(lhs, {}),
                         TRANSIENT);
            return noobj();
          })
          ->create(),
          InstBuilder::build(driver_value_id.extend(":set_clock"))
          ->type_args(x(0, "frequency", jnt(80)))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(id_p(driver_remote_id->extend(":set_clock")),
                         ObjHelper::make_lhs_args(lhs, args),
                         TRANSIENT);
            return noobj();
          })
          ->create()
      });
      Type::singleton()->save_type(
          id_p("/lib/driver/i2c/arduino/furi"),
          rec({{vri(":create"),
                InstBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":create"))
                ->type_args(
                    x(0, "local_id", vri(driver_value_id)),
                    x(1, "remote_id", vri(driver_remote_id)),
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

    static Obj_p load_local(const ID &driver_value_id, const ID_p &driver_remote_id,
                            const ID &define_ns_prefix = ID("i2c")) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":begin"))
          ->instance_f([](const Obj_p &, const InstArgs &) {
            Wire.begin();
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":end"))
          ->instance_f([](const Obj_p &, const InstArgs &) {
            Wire.end();
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":request_from"))
          ->type_args(x(0, "address"), x(1, "quantity"))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            Wire.requestFrom(args.at(0)->int_value(),args.at(1)->int_value());
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":begin_transmission"))
          ->type_args(x(0, "address"))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            Wire.beginTransmission(args.at(0)->int_value());
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":end_transmission"))
          ->type_args(x(0, "stop", dool(true)))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            Wire.endTransmission(args.at(0)->bool_value());
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":write"))
          ->type_args(x(0, "data", str("")))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            Wire.write(args.at(0)->str_value().c_str());
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":available"))
          ->instance_f([](const Obj_p &, const InstArgs &) {
            return jnt(Wire.available());
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":read"))
          ->instance_f([](const Obj_p &, const InstArgs &) {
            return jnt(Wire.read());
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":set_clock"))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            Wire.setClock(args.at(0)->int_value());
            return noobj();
          })
          ->create()
      });
      //////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////// ARDUINO DRIVER INSTALLATION ///////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
      id_p("/lib/driver/i2c/arduino/pin"),
          rec({{vri(":create"),
                ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":create"))
                ->type_args(x(0, "local_in", vri(driver_value_id)),
                            x(1, "remote_id", vri(driver_remote_id)),
                            x(2, "ns_prefix", vri(define_ns_prefix)))
                ->instance_f([inst_types](const Obj_p &lhs, const InstArgs &args) {
                  const Rec_p record = rec();
                  for (const auto &i: *inst_types) {
                    record->rec_set(vri(i->inst_op()), i);
                  }
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":begin"),
                      Obj::to_bcode([args](const Obj_p &) {
                        Wire.begin();
                        return noobj();
                      })));
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":end"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        Wire.end();
                        return noobj();
                      })));
                 ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":request_from"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        const Int_p result = jnt(
                            Wire.requestFrom(args.at(0)->int_value(), args.at(1)->int_value()));
                        ROUTER_WRITE(id_p(args.at(1)->uri_value().extend(":request_from")), result,RETAIN);
                        return result;
                      })));
                 ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":begin_transmission"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        Wire.beginTransmission(args.at(0)->int_value());
                        return noobj();
                      })));
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":end_transmission"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        Wire.endTransmission(args.at(0)->bool_value());
                        return noobj();
                      })));
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":write"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        Wire.write(args.at(0)->str_value().c_str());
                        return noobj();
                      })));
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":available"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        const Int_p result = jnt(Wire.available());
                        ROUTER_WRITE(id_p(args.at(1)->uri_value().extend(":available")), result,RETAIN);
                        return result;
                      })));
                 ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":read"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        const Int_p result = jnt(Wire.read());
                        ROUTER_WRITE(id_p(args.at(1)->uri_value().extend(":write")), result,RETAIN);
                        return result;
                      })));
                  ROUTER_SUBSCRIBE(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":set_clock"),
                      Obj::to_bcode([lhs, args](const Obj_p &) {
                        Wire.setClock(args.at(0)->int_value());
                        return noobj();
                      })));
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
}
#endif