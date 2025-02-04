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
#ifndef fhatos_arduino_pwm_hpp
#define fhatos_arduino_pwm_hpp

#include "../../../fhatos.hpp"
#include "../../../lang/type.hpp"
#include "../../../lang/obj.hpp"
#include "../../../util/obj_helper.hpp"
#include "../../../structure/router.hpp"
#ifdef ARDUINO
#include <Arduino.h>
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif
//#ifdef NATIVE
//#include <wiringPi.h>
//#endif

namespace fhatos {
  class ArduinoPWM final : public Rec {
  public:
    explicit ArduinoPWM(const ID &value_id) : Rec(
      rmap({
        {"write",
          InstBuilder::build(value_id.extend("write"))
          ->domain_range(INT_FURI, {0, 1}, INT_FURI, {0, 1})
          ->inst_args(rec({
            {"pin", Obj::to_type(INT_FURI)},
            {"value", Obj::to_type(BOOL_FURI)}
          }))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const uint8_t pin = args->arg("pin")->int_value();
            const uint8_t value = args->arg("value")->int_value();
            pinMode(pin, OUTPUT);
            analogWrite(pin, value);
            return lhs;
          })
          ->create()},
        {"read",
          InstBuilder::build(value_id.extend("read"))
          ->domain_range(INT_FURI, {0, 1}, INT_FURI, {1, 1})
          ->inst_args(rec({{"pin", Obj::to_type(INT_FURI)}}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return jnt(analogRead(args->arg("pin")->int_value()));
          })
          ->create()}}), OType::REC, REC_FURI, id_p(value_id)) {
    }

    static ptr<ArduinoPWM> create(const ID &id) {
      const auto pwms = std::make_shared<ArduinoPWM>(id);
      Router::singleton()->subscribe(
       Subscription::create(id_p(id), p_p(id.extend("+")),
                            InstBuilder::build(id.extend("s_write"))
                            ->inst_args(rec({
                              {"target", Obj::to_bcode()},
                              {"payload", Obj::to_bcode()},
                              {"retain", Obj::to_bcode()}}))
                            ->inst_f([id](const Obj_p &lhs, const InstArgs &args) {
                              const Int_p pin = jnt(stoi(args->arg("target")->uri_value().name()));
                              return Obj::to_inst(to_inst_args({pin, lhs}), id_p(id.extend("write")))->apply(lhs);
                            })->create(id_p(id.extend("s_write")))));
      return pwms;
    }

    static void *import(const ID &lib_id = "/io/lib/pwm") {
      //Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_args(rec({{"value_id", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return make_shared<Obj>(ArduinoPWM(args->arg("value_id")->uri_value()));
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
