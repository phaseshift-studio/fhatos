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
#ifndef fhatos_pwm_hpp
#define fhatos_pwm_hpp

#ifndef NATIVE
#include "../../../../fhatos.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../../structure/router.hpp"
//
#ifdef ARDUINO
#include <Arduino.h>
#elif defined(RASPBERRYPI)
#include <wiringPi.h>
#endif

namespace fhatos {
  static ID_p PWM_FURI = id_p("/fos/io/pwm");

  class PWM final {
  public:
    static void *import() {
      Typer::singleton()->save_type(*PWM_FURI, Obj::to_type(INT_FURI));
      InstBuilder::build(PWM_FURI->add_component("write"))
          ->domain_range(PWM_FURI, {1, 1}, PWM_FURI, {1, 1})
          ->inst_args(rec({{"value", Obj::to_type(INT_FURI)}}))
          ->inst_f([](const Obj_p &gpio, const InstArgs &args) {
            const uint8_t pin = gpio->int_value();
            const uint8_t value = args->arg("value")->int_value();
            pinMode(pin, OUTPUT);
            analogWrite(pin, value);
            return gpio;
          })->save();
      ///////////////////////////////////////////////////////
      InstBuilder::build(PWM_FURI->add_component("read"))
          ->domain_range(PWM_FURI, {1, 1}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &pwm, const InstArgs &) {
            return jnt(analogRead(pwm->int_value()));
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
#endif
