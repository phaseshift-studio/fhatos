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
#include <language/type.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  enum class PROTOCOL { PWM, GPIO, I2C, SPI, MQTT };

  static const auto ProtocolTypes = Enums<PROTOCOL>({{PROTOCOL::GPIO, "gpio"},
                                                     {PROTOCOL::PWM, "pwm"},
                                                     {PROTOCOL::I2C, "i2c"},
                                                     {PROTOCOL::SPI, "spi"},
                                                     {PROTOCOL::MQTT, "mqtt"}});

  // static const ID_p DRIVER_TYPE = id_p(FOS_TYPE_PREFIX "/rec/driver/");
  static const ID_p PROTOCOL_TYPE_PREFIX = id_p(FOS_TYPE_PREFIX "/uri/protocol/");
  static const auto DRIVER_FURI = make_shared<ID>(FOS_TYPE_PREFIX "/rec/driver/");

  class fDriver : public Rec {

  public:
    explicit fDriver(const List_p<Inst_p> &instructions, const ID_p &type, const ID_p &id) :
        Rec(make_shared<RecMap<>>(), type) {
      for (const auto &inst: *instructions) {
        rec_set(vri(inst->inst_op()), inst);
      }
      this->id_ = id;
    }
#ifdef ESP_ARCH
    static ptr<fDriver> gpio(const ID &id) {
      Type::singleton()->save_type(id_p(DRIVER_FURI->extend("gpio")), Obj::to_bcode());
      return make_shared<fDriver>(
          make_shared<List<Inst_p>>(List<Inst_p>{Obj::to_inst(
                                                     ":digital_write", {x(0), x(1)},
                                                     [](const InstArgs &args) {
                                                       return [args](const Obj_p &lhs) {
                                                         const uint8_t pin = args.at(0)->apply(lhs)->int_value();
                                                         const Int_p value = args.at(1)->apply(lhs);
                                                         pinMode(pin, OUTPUT);
                                                         digitalWrite(pin, value->int_value());
                                                         return noobj();
                                                       };
                                                     },
                                                     IType::ONE_TO_ZERO),
                                                 Obj::to_inst(
                                                     ":digital_read", {x(0)},
                                                     [](const InstArgs &args) {
                                                       return [args](const Obj_p &lhs) {
                                                         return jnt(digitalRead(args.at(0)->apply(lhs)->int_value()));
                                                       };
                                                     },
                                                     IType::ONE_TO_ONE)}),
          id_p(DRIVER_FURI->extend("gpio")), id_p(id));
    }
#endif
  };
  using fDriver_p = shared_ptr<fDriver>;
} // namespace fhatos
#endif