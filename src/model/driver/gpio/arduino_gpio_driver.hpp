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
#ifndef fhatos_arduino_gpio_driver_hpp
#define fhatos_arduino_gpio_driver_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/parser.hpp>
#include <structure/router.hpp>

#include <model/driver/fdriver.hpp>

namespace fhatos {
  class ArduinoGPIODriver {
  protected:
    static fDriver_p load_furi_driver(const ID_p &request_id, const ID_p &response_id) {
      const auto driver = make_shared<fDriver>(
        request_id, response_id, ptr<List<Inst_p>>(new List<Inst_p>({
          Obj::to_inst(
            "gpio:digital_write", {x(0), x(1)},
            [request_id](const InstArgs &args) {
              return [request_id, args](const Obj_p &lhs) {
                router()->write(request_id,
                                parse("gpio:digital_write(%i,%i)", args.at(0)->apply(lhs)->int_value(),
                                      args.at(1)->apply(lhs)->int_value()));
                return noobj();
              };
            },
            IType::ONE_TO_ZERO),
          Obj::to_inst(
            "gpio:digital_read", {x(0)},
            [response_id, request_id](const InstArgs &args) {
              return [response_id, request_id, args](const Obj_p &lhs) {
                router()->write(request_id, parse("gpio:digital_read(%i)", args.at(0)->apply(lhs)->int_value()));
                return router()->read(response_id);
              };
            },
            IType::ONE_TO_ONE),
        })),
        id_p(DRIVER_FURI->resolve("./gpio/arduino/furi")));
      return driver;
    }

#ifdef ESP_ARCH
    static fDriver_p load_hardware_driver(const ID_p &request_id, const ID_p &response_id) {
      const auto driver = make_shared<fDriver>(request_id, response_id,
                                               ptr<List<Inst_p>>(new List<Inst_p>({
                                                 Obj::to_inst(
                                                   "gpio:digital_write", {x(0), x(1)},
                                                   [response_id](const InstArgs &args) {
                                                     return [response_id,args](const Obj_p &lhs) {
                                                       const uint8_t pin = args.at(0)->apply(lhs)->int_value();
                                                       const Int_p value = args.at(1)->apply(lhs);
                                                       pinMode(pin, OUTPUT);
                                                       digitalWrite(pin, value->int_value());
                                                       router()->write(response_id, value);
                                                       return noobj();
                                                     };
                                                   },
                                                   IType::ONE_TO_ZERO),
                                                 Obj::to_inst(
                                                   "gpio:digital_read", {x(0)},
                                                   [response_id](const InstArgs &args) {
                                                     return [response_id, args](const Obj_p &lhs) {
                                                       const Int_p result = jnt(
                                                         digitalRead(args.at(0)->apply(lhs)->int_value()));
                                                       router()->write(response_id, result);
                                                       return result;
                                                     };
                                                   },
                                                   IType::ONE_TO_ONE),
                                               })),
                                               id_p(DRIVER_FURI->resolve("./gpio/arduino/hardware")));
      router()->route_subscription(subscription_p(*driver->type(),
                                                  *request_id, Subscription::to_bcode(
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
