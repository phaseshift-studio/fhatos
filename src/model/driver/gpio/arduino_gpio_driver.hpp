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
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":digital_write"))
          ->type_args(
              x(0, "pin"),
              x(1, "value"),
              x(2, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const Obj_p &lhs, const InstArgs &args) {
            router()->write(id_p(args.at(2)->uri_value().extend(":digital_write")),
                            ObjHelper::make_lhs_args(lhs, {args.at(0), args.at(1)}),
                            TRANSIENT);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":digital_read"))
          ->type_args(
              x(0, "pin"),
              x(1, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const Obj_p &lhs, const InstArgs &args) {
            const ID_p &inst_id = id_p(args.at(1)->uri_value().extend(":digital_read"));
            router()->write(inst_id, ObjHelper::make_lhs_args(lhs, {args.at(0)}), TRANSIENT);
            return router()->read(inst_id);
          })
          ->create()});
      //////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////// FURI DRIVER INSTALLATION //////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
          GPIO_ARDUINO_FURI_TYPE,
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

#ifdef ESP_ARCH
    static Obj_p load_local(const ID &driver_value_id, const ID_p &driver_remote_id,
                            const ID &define_ns_prefix = ID("gpio")) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":digital_write"))
          ->type_args(
              x(0, "pin"),
              x(1, "value"),
              x(2, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            const uint8_t pin = args.at(0)->int_value();
            const uint8_t value = args.at(1)->int_value();
            pinMode(pin, OUTPUT);
            digitalWrite(pin, value);
            return noobj();
          })
          ->create(),
          ObjHelper::InstTypeBuilder::build(driver_value_id.extend(":digital_read"))
          ->type_args(
              x(0, "pin"),
              x(1, "driver_remote_id", vri(driver_remote_id)))
          ->instance_f([](const Obj_p &, const InstArgs &args) {
            return jnt(digitalRead(args.at(0)->int_value()));
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
                            x(1, "driver_remote_id", vri(driver_remote_id)),
                            x(2, "ns_prefix", vri(define_ns_prefix)))
                ->instance_f([inst_types](const Obj_p &lhs, const InstArgs &args) {
                  const Rec_p record = rec();
                  for (const auto &i: *inst_types) {
                    record->rec_set(vri(i->inst_op()), i);
                  }
                  router()->route_subscription(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":digital_write"),
                      Obj::to_bcode([args](const Obj_p &message) {
                        LHSArgs_p lhs_args = ObjHelper::parse_lhs_args(message);
                        const uint8_t pin = lhs_args->second->at(0)->int_value();
                        const uint8_t value = lhs_args->second->at(1)->int_value();
                        pinMode(pin, OUTPUT);
                        digitalWrite(pin, value);
                        return noobj();
                      })));
                  router()->route_subscription(Subscription::create(
                      args.at(0)->uri_value(), args.at(1)->uri_value().extend(":digital_read"),
                      Obj::to_bcode([lhs, args](const Obj_p &message) {
                        LOG(ERROR, "processing input %s\n", message->toString().c_str());
                        //LOG(ERROR, "processing message %i\n", );
                        const LHSArgs_p lhs_args = ObjHelper::parse_lhs_args(message);
                        //if (!lhs_args->second->at(0)->rec_get(":retain")->bool_value()) {
                        const uint8_t pin = lhs_args->second->at(0)->int_value();
                        LOG(ERROR,"reading pin %i\n",pin);
                        //pinMode(pin, INPUT);
                        const int value = digitalRead(pin);
                        LOG(ERROR,"writting pin value %i\n",value);
                        router()->write(id_p(args.at(1)->uri_value().extend(":digital_read")),
                                        jnt(value), RETAIN);
                        return jnt(value);
                        //}
                        // return noobj();
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
    }
#endif
  };
} // namespace fhatos
#endif