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
#include <structure/router.hpp>
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <model/driver/driver.hpp>

namespace fhatos {
  class ArduinoGPIODriver {
  public:
    static Obj_p load_remote(const ID &driver_value_id, const ID_p &driver_remote_id,
                             const ID &define_ns_prefix = ID("gpio")) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_write"))
          ->type_args(x(0, "pin"), x(1, "value"), x(2, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
            router()->write(id_p(args.at(2)->apply(lhs)->uri_value().extend(":digital_write")),
                            ObjHelper::make_lhs_args(lhs, {args.at(0)->apply(lhs), args.at(1)->apply(lhs)}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_read"))
          ->type_args(x(0, "pin"), x(1, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
            router()->write(id_p(args.at(1)->apply(lhs)->uri_value().extend(":digital_read")),
                            ObjHelper::make_lhs_args(lhs, {args.at(0)->apply(lhs)}), TRANSIENT);
            return router()->read(id_p(args.at(1)->apply(lhs)->uri_value().extend(":digital_read")));
          })
          ->create()});
      //////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////// FURI DRIVER INSTALLATION //////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
          GPIO_ARDUINO_FURI_TYPE,
          rec({{vri(":install"),
                ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":install"))
                ->type_args(x(0, "install_location", vri(driver_value_id)),
                            x(1, "driver_remote_id", vri(driver_remote_id)),
                            x(2, "ns_prefix", vri(define_ns_prefix)))
                ->instance_f([inst_types](const InstArgs &args, const Obj_p &lhs) {
                  const Rec_p record = rec();
                  for (const auto &i: *inst_types) {
                    record->rec_set(vri(i->inst_op()), i);
                  }
                  const Uri_p driver_id = args.at(0)->apply(lhs);
                  const Uri_p ns_prefix = args.at(2)->apply(lhs);
                  if (!ns_prefix->is_noobj())
                    Type::singleton()->save_type(id_p(ID("/type/uri/ns/prefix/").extend(ns_prefix->uri_value())),
                                                 driver_id);
                  return record->at(id_p(args.at(0)->apply(lhs)->uri_value()));
                })
                ->create()}}));
      return noobj();
    }

#ifdef ESP_ARCH
    static Obj_p load_local(const ID &driver_value_id, const ID_p &driver_remote_id) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_write"))
              ->type_args(x(0, "pin"), x(1, "value"), x(2, "driver_remote_id", vri(driver_remote_id)))
              ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
                const uint8_t pin = args.at(0)->apply(lhs)->int_value();
                const uint8_t value = args.at(1)->apply(lhs)->int_value();
                pinMode(pin, OUTPUT);
                digitalWrite(pin, value);
                // router()->write(id_p(driver_remote_id->extend(":digital_write")),
                //                 lst({lhs, lst({args.at(0)->apply(lhs), args.at(1)->apply(lhs)})}), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_read"))
              ->type_args(x(0, "pin"), x(1, "driver_remote_id", vri(driver_remote_id)))
              ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
                return jnt(digitalRead(args.at(0)->apply(lhs)->int_value()));
                // router()->write(id_p(driver_remote_id->extend(":digital_read")),
                //                 lst({lhs, lst({args.at(0)->apply(lhs)})}), TRANSIENT);
                return noobj();
              })
              ->create()});
      //////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////// ARDUINO DRIVER INSTALLATION ///////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
          GPIO_ARDUINO_PIN_TYPE,
          rec({{vri(":install"),
                ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":install"))
                    ->type_args(x(0, "install_location", vri(driver_value_id)),
                                x(1, "driver_remote_id", vri(driver_remote_id)))
                    ->instance_f([inst_types](const InstArgs &args, const Obj_p &lhs) {
                      const Rec_p record = rec();
                      for (const auto &i: *inst_types) {
                        record->rec_set(vri(i->inst_op()), i);
                      }
                      router()->route_subscription(
                          Subscription::create(args.at(0)->uri_value(), args.at(1)->uri_value().extend(":digital_write"),
                                         Subscription::to_bcode([args](const Message_p &message) {
                                           const uint8_t pin = message->arg(0)->int_value();
                                           const uint8_t value = message->arg(1)->int_value();
                                           pinMode(pin, OUTPUT);
                                           digitalWrite(pin, value);
                                         })));
                      router()->route_subscription(Subscription::create(
                          args.at(0)->uri_value(), args.at(1)->apply(lhs)->uri_value().extend(":digital_read"),
                          Subscription::to_bcode([lhs, args](const Message_p &message) {
                            if (!message->retain) {
                              const uint8_t pin = message->arg(0)->int_value();
                              pinMode(pin, INPUT);
                              const int value = digitalRead(pin);
                              router()->write(id_p(args.at(1)->apply(lhs)->uri_value().extend(":digital_read")),
                                              jnt(value), RETAIN);
                            }
                          })));
                      return record->at(id_p(args.at(0)->apply(lhs)->uri_value()));
                    })
                    ->create()}}));
      return noobj();
    }
#endif
  };
} // namespace fhatos
#endif