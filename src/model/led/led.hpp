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
#ifndef fhatos_led_hpp
#define fhatos_led_hpp

#include <fhatos.hpp>
#include <structure/stype/computed.hpp>

namespace fhatos {
  class Led : public Computed {
  protected:
    Pattern_p pwm_pattern_;

    explicit Led(const Pattern &pattern = "/ui/led/#", const Pattern &pwm_pattern = "/soc/pwm/#") : Computed(pattern),
      pwm_pattern_(p_p(pwm_pattern)) {
      this->read_functions_->insert({p_p("/ui/led/+"), [this](const fURI_p &furi) -> ReadRawResult_p {
        return make_shared<ReadRawResult>(ReadRawResult{
          {{id_p(*furi), router()->read(furi_p(this->pwm_pattern_->resolve(string("./") + furi->name())))}}
        });
      }});
      this->write_functions_->insert({p_p("/ui/led/+"), [this](const fURI_p &furi, const Obj_p &obj) -> ReadRawResult {
        router()->write(furi_p(this->pwm_pattern_->resolve(string("./") + furi->name())), obj,RETAIN);
        return {};
      }});
    }

  public:
    static ptr<Led> create(const Pattern &pattern = "/ui/led/#", const Pattern &pwm_pattern = "/soc/pwm/#") {
      const auto gpio = ptr<Led>(new Led(pattern, pwm_pattern));
      return gpio;
    }
  };
} // namespace fhatos
#endif
