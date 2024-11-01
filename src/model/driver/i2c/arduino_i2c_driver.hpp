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

#include <model/driver/fdriver.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#endif

namespace fhatos {
  class ArduinoI2CDriver {
  protected:
    static fDriver_p load_furi_driver(const ID_p &request_id, const ID_p &response_id) {
      const fDriver_p driver = make_shared<fDriver>(request_id, response_id, ptr<List<Inst_p>>(new List<Inst_p>({
                                                        Obj::to_inst(
                                                            "i2c:begin", {},
                                                            [request_id](const InstArgs &args) {
                                                              return [request_id,args](const Obj_p &) {
                                                                router()->write(request_id, parse("i2c:begin()"));
                                                                return noobj();
                                                              };
                                                            },
                                                            IType::ONE_TO_ZERO),
                                                        Obj::to_inst(
                                                            "i2c:end", {},
                                                            [request_id](const InstArgs &args) {
                                                              return [request_id,args](const Obj_p &) {
                                                                router()->write(request_id, parse("i2c:end()"));
                                                                return noobj();
                                                              };
                                                            },
                                                            IType::ONE_TO_ZERO),

                                                        Obj::to_inst(
                                                            "i2c:request_from", {x(0), x(1)},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &lhs) {
                                                                router()->write(
                                                                    request_id, parse("i2c:request_from(%i,%i)",
                                                                      args.at(0)->apply(lhs)->int_value(),
                                                                      args.at(1)->apply(lhs)->int_value()));
                                                                return router()->read(response_id);
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE),

                                                        Obj::to_inst(
                                                            "i2c:begin_transmission", {x(0)},
                                                            [request_id](const InstArgs &args) {
                                                              return [request_id, args](const Obj_p &lhs) {
                                                                router()->write(
                                                                    request_id, parse("i2c:begin_transmission(%i)",
                                                                      args.at(0)->apply(lhs)->int_value()));
                                                                return noobj();
                                                              };
                                                            },
                                                            IType::ONE_TO_ZERO),

                                                        Obj::to_inst(
                                                            "i2c:end_transmission", {x(0, dool(true))},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &lhs) {
                                                                router()->write(
                                                                    request_id, parse("i2c:end_transmission(%s)",
                                                                      args.at(0)->apply(lhs)->bool_value()
                                                                        ? "true"
                                                                        : "false"));
                                                                return router()->read(response_id);
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE),
                                                        Obj::to_inst(
                                                            "i2c:write", {x(0)},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &lhs) {
                                                                router()->write(
                                                                    request_id, parse("i2c:write(%i)",
                                                                      args.at(0)->apply(lhs)->int_value()));
                                                                return router()->read(response_id);
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE),

                                                        Obj::to_inst(
                                                            "i2c:available", {},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &) {
                                                                router()->write(request_id, parse("i2c:available()"));
                                                                return router()->read(response_id);
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE),

                                                        Obj::to_inst(
                                                            "i2c:read", {},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &) {
                                                                router()->write(request_id, parse("i2c:read()"));
                                                                return router()->read(response_id);
                                                              };
                                                            },
                                                            IType::ONE_TO_ONE),

                                                        Obj::to_inst(
                                                            "i2c:set_clock", {x(0)},
                                                            [response_id,request_id](const InstArgs &args) {
                                                              return [response_id,request_id,args](const Obj_p &lhs) {
                                                                router()->write(
                                                                    request_id, parse("i2c:set_clock(%i)",
                                                                      args.at(0)->apply(lhs)->int_value()));
                                                                return noobj();
                                                              };
                                                            },
                                                            IType::ONE_TO_ZERO)
                                                    })), id_p(DRIVER_FURI->resolve("i2c/arduino/furi")));
      return driver;
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
         router()->route_subscription(subscription_p(*driver->type(),
                                                  *request_id, Insts::to_bcode(
                                                    [response_id](const Message_p &message) {
                                                      const Obj_p result = message->payload->apply(noobj());
                                                      if (!result->is_noobj())
                                                        router()->write(response_id, result);
                                                    })));
      return driver;
      }
#endif

  public:
    static fDriver_p create(const ID &request_id, const ID &response_id) {
#ifdef NATIVE
      return load_furi_driver(id_p(request_id), id_p(response_id));
#elif defined(ESP_ARCH)
        return load_hardware_driver(id_p(request_id),id_p(response_id));
#endif
    }
  };
} // namespace fhatos
#endif