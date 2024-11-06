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
#ifndef fhatos_fdriver_hpp
#define fhatos_fdriver_hpp

#include <language/obj.hpp>
#include <language/parser.hpp>
#include <util/obj_helper.hpp>

#define LOG_DRIVER(logtype, driver, format, ...)                                                                       \
  LOG((logtype), (string("!G[!Y%s!G]!! ") + (format)).c_str(), (driver)->id()->toString().c_str(), ##__VA_ARGS__)

namespace fhatos {
  enum class PROTOCOL { PWM, GPIO, I2C, SPI, MQTT };

  static const auto ProtocolTypes = Enums<PROTOCOL>({{PROTOCOL::GPIO, "gpio"},
                                                     {PROTOCOL::PWM, "pwm"},
                                                     {PROTOCOL::I2C, "i2c"},
                                                     {PROTOCOL::SPI, "spi"},
                                                     {PROTOCOL::MQTT, "mqtt"}});

  // static const ID_p DRIVER_TYPE = id_p(FOS_TYPE_PREFIX "/rec/driver/");
  static const ID_p PROTOCOL_TYPE_PREFIX = id_p(FOS_TYPE_PREFIX "/uri/protocol/");
  static const auto DRIVER_REC_FURI = id_p(FOS_TYPE_PREFIX "/rec/driver/");
  static const auto DRIVER_INST_FURI = id_p(FOS_TYPE_PREFIX "/inst/driver/");
  static const auto GPIO_ARDUINO_PIN_TYPE = id_p(DRIVER_REC_FURI->resolve("./gpio/arduino/pin"));
  static const auto GPIO_ARDUINO_FURI_TYPE = id_p(DRIVER_REC_FURI->resolve("./gpio/arduino/furi"));

  class fDriver {

    explicit fDriver() = default;

  public:
    static Obj_p gpio_furi(const ID &driver_value_id, const ID_p &driver_remote_id) {
      const auto inst_types = make_shared<List<Inst_p>>(List<Inst_p>{
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_write"))
              ->type_args(x(0, "pin"), x(1, "value"), x(2, "driver_remote_id", vri(driver_remote_id)))
              ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
                router()->write(id_p(args.at(2)->apply(lhs)->uri_value().extend(":digital_write")),
                                lst({lhs, lst({args.at(0)->apply(lhs), args.at(1)->apply(lhs)})}), TRANSIENT);
                return noobj();
              })
              ->create(),
          ObjHelper::InstTypeBuilder::build(DRIVER_INST_FURI->extend(driver_value_id).extend(":digital_read"))
              ->type_args(x(0, "pin"), x(1, "driver_remote_id", vri(driver_remote_id)))
              ->instance_f([](const InstArgs &args, const Obj_p &lhs) {
                router()->write(id_p(args.at(1)->apply(lhs)->uri_value().extend(":digital_read")),
                                lst({lhs, lst({args.at(0)->apply(lhs)})}), TRANSIENT);
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
                                x(1, "driver_remote_id", vri(driver_remote_id)))
                    ->instance_f([inst_types](const InstArgs &args, const Obj_p &lhs) {
                      const Rec_p record = rec();
                      for (const auto &i: *inst_types) {
                        record->rec_set(vri(i->inst_op()), i);
                      }
                      return record->at(id_p(args.at(0)->apply(lhs)->uri_value()));
                    })
                    ->create()}}));
      return noobj();
    }

#ifdef ESP_ARCH
    static Obj_p gpio_pin(const ID &driver_value_id, const ID_p &driver_remote_id) {
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
                          subscription_p(args.at(0)->uri_value(), args.at(1)->uri_value().extend(":digital_write"),
                                         Subscription::to_bcode([args](const Message_p &message) {
                                           const uint8_t pin = message->payload->lst_get(1)->lst_get(0)->int_value();
                                           const uint8_t value = message->payload->lst_get(1)->lst_get(1)->int_value();
                                           pinMode(pin, OUTPUT);
                                           digitalWrite(pin, value);
                                         })));
                      router()->route_subscription(subscription_p(
                          args.at(0)->uri_value(), args.at(1)->apply(lhs)->uri_value().extend(":digital_read"),
                          Subscription::to_bcode([lhs, args](const Message_p &message) {
                            if (message->retain)
                              return;
                            const uint8_t pin = message->payload->lst_get(1)->lst_get(0)->int_value();
                            pinMode(pin, OUTPUT);
                            const uint16_t value = digitalRead(pin);
                            router()->write(id_p(args.at(1)->apply(lhs)->uri_value().extend(":digital_read")),
                                            jnt(value), RETAIN);
                          })));
                      return record->at(id_p(args.at(0)->apply(lhs)->uri_value()));
                    })
                    ->create()}}));
      return noobj();
    }
#endif
  };

  using fDriver_p = shared_ptr<fDriver>;
} // namespace fhatos
#endif