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
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":begin"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":begin")),
                            ObjHelper::make_lhs_args(lhs, {}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":end"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":end")),
                            ObjHelper::make_lhs_args(lhs, {}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":request_from"))
          ->type_args(x(0, "address"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":request_from")),
                            ObjHelper::make_lhs_args(lhs, args),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":begin_transmission"))
          ->type_args(x(0, "address"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":request_from")),
                            ObjHelper::make_lhs_args(lhs, args),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":end_transmission"))
          ->type_args(x(0, "stop", dool(true)))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":end_transmission")),
                            ObjHelper::make_lhs_args(lhs, args),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":write"))
          ->type_args(x(0, "data array", str("")))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":write")),
                            ObjHelper::make_lhs_args(lhs, args),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":available"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            router()->write(id_p(driver_remote_id->extend(":available")),
                            ObjHelper::make_lhs_args(lhs, {}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":read"))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &) {
            router()->write(id_p(driver_remote_id->extend(":read")),
                            ObjHelper::make_lhs_args(lhs, {}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":set_clock"))
          ->type_args(x(0, "frequency", jnt(80)))
          ->instance_f([driver_remote_id](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(driver_remote_id->extend(":set_clock")),
                            ObjHelper::make_lhs_args(lhs, args),
                            TRANSIENT);
            return noobj();
          })
          ->create()
      });
      Type::singleton()->save_type(
        id_p(DRIVER_REC_FURI->resolve("./i2c/arduino/furi")),
        rec({{vri(":install"),
              ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":install"))
              ->type_args(
                  x(0, "install_location", vri(driver_value_id)),
                  x(1, "driver_remote_id", vri(driver_remote_id)),
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
  };
}



///////////////////////////////////////////////////////
/////////////// ARDUINO HARDWARE DRIVER ///////////////
///////////////////////////////////////////////////////
#ifdef ESP_ARCH
      static fDriver_p load_hardware_driver(const ID_p &request_id, const ID_p &response_id) {
        const auto driver = make_shared<fDriver>(request_id, response_id, ptr<List<Inst_p>>(new List<Inst_p>({
                                                    Obj::to_inst(
                                                        "i2c:begin", {},
                                                        [](const InstArgs &) {
                                                          return [](const Obj_p &) {
                                                            Wire.begin();
                                                            return noobj();
                                                          };
                                                        },
                                                        IType::ONE_TO_ZERO),
                                                    Obj::to_inst(
                                                        "i2c:end", {},
                                                        [](const InstArgs &) {
                                                          return [](const Obj_p &) {
                                                            Wire.end();
                                                            return noobj();
                                                          };
                                                        },
                                                        IType::ONE_TO_ZERO),

                                                    Obj::to_inst(
                                                        "i2c:request_from", {x(0), x(1)},
                                                        [response_id](const InstArgs &args) {
                                                          return [response_id,args](const Obj_p &lhs) {
                                                            const Int_p result = jnt(
                                                                Wire.requestFrom(
                                                                    args.at(0)->apply(lhs)->int_value(),
                                                                    args.at(1)->apply(lhs)->int_value()));
                                                            router()->write(response_id, result);
                                                            return result;
                                                          };
                                                        },
                                                        IType::ONE_TO_ONE),

                                                    Obj::to_inst(
                                                        "i2c:begin_transmission", {x(0)},
                                                        [](const InstArgs &args) {
                                                          return [args](const Obj_p &lhs) {
                                                            Wire.beginTransmission(args.at(0)->apply(lhs)->int_value());
                                                            return noobj();
                                                          };
                                                        },
                                                        IType::ONE_TO_ZERO),

                                                    Obj::to_inst(
                                                        "i2c:end_transmission", {x(0,dool(true))},
                                                        [response_id](const InstArgs &args) {
                                                          return [response_id,args](const Obj_p &lhs) {
                                                            const Int_p result = jnt(
                                                                Wire.endTransmission(
                                                                    args.at(0)->apply(lhs)->bool_value()));
                                                            router()->write(response_id, result);
                                                            return result;
                                                          };
                                                        },
                                                        IType::ONE_TO_ONE),
                                                    Obj::to_inst(
                                                        "i2c:write", {x(0)},
                                                        [response_id](const InstArgs &args) {
                                                          return [response_id,args](const Obj_p &lhs) {
                                                            const Int_p result = jnt(
                                                                Wire.write(args.at(0)->apply(lhs)->int_value()));
                                                            router()->write(response_id, result);
                                                            return result;
                                                          };
                                                        },
                                                        IType::ONE_TO_ONE),

                                                    Obj::to_inst(
                                                        "i2c:available", {},
                                                        [response_id](const InstArgs &args) {
                                                          return [response_id,args](const Obj_p &) {
                                                            const Int_p result = jnt(
                                                                Wire.available());
                                                            router()->write(response_id, result);
                                                            return result;
                                                          };
                                                        },
                                                        IType::ONE_TO_ONE),

                                                    Obj::to_inst(
                                                        "i2c:read", {},
                                                        [response_id](const InstArgs &args) {
                                                          return [response_id,args](const Obj_p &) {
                                                            const Int_p result = jnt(Wire.read());
                                                            router()->write(response_id, result);
                                                            return result;
                                                          };
                                                        },
                                                        IType::ONE_TO_ONE),
                                                    Obj::to_inst(
                                                        "i2c:set_clock", {x(0)},
                                                        [](const InstArgs &args) {
                                                          return [args](const Obj_p &lhs) {
                                                            Wire.setClock(args.at(0)->apply(lhs)->int_value());
                                                            return noobj();
                                                          };
                                                        },
                                                        IType::ONE_TO_ZERO)
                                                })),  id_p(DRIVER_FURI->resolve("i2c/arduino/furi")));
         router()->route_subscription(Subscription::create(*driver->type(),
                                                  *request_id, Insts::to_bcode(
                                                    [response_id](const Message_p &message) {
                                                      const Obj_p result = message->payload->apply(noobj());
                                                      if (!result->is_noobj())
                                                        router()->write(response_id, result);
                                                    })));
      return driver;
      }
#endif
#endif