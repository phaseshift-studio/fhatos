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
#ifndef fhatos_interrupt_hpp
#define fhatos_interrupt_hpp

#include <fhatos.hpp>
#include <model/pin/pin.hpp>

// using namespace fhatos;

namespace fhatos {
  template<typename PIN_DRIVER>
  class Interrupt : public Pin<PIN_DRIVER> {
  protected:
    Map_p<uint8_t, BCode_p> interrupts_;

    explicit Interrupt(
      const Pattern &pattern,
      const ptr<PIN_DRIVER> &driver = nullptr) : Pin<PIN_DRIVER>(pattern,
                                                                 [this](const uint8_t pin) -> BCode_p {
                                                                   return this->interrupts_->count(pin)
                                                                            ? this->interrupts_->at(pin)
                                                                            : noobj();
                                                                 },
                                                                 [this](const uint8_t pin,
                                                                        const BCode_p &bcode) -> void {
                                                                   if (this->interrupts_->count(pin)) {
                                                                     this->interrupts_->erase(pin);
                                                                     this->driver_->set_interrupt(
                                                                       pin, id_p(this->pattern_->resolve(
                                                                         string("./") + to_string(pin))), noobj());
                                                                     LOG_STRUCTURE(
                                                                       INFO, this,
                                                                       "!bpin %i!! !yinterrupt!! detached\n", pin);
                                                                   }
                                                                   if (!bcode->is_noobj()) {
                                                                     const BCode_p bclone = bcode->clone();
                                                                     this->interrupts_->insert({pin, bclone});
                                                                     this->driver_->set_interrupt(
                                                                       pin, id_p(this->pattern_->resolve(
                                                                         string("./") + to_string(pin))), bclone);
                                                                     LOG_STRUCTURE(
                                                                       INFO, this,
                                                                       "!bpin %i!! !yinterrupt!! attached\n", pin);
                                                                   }
                                                                 }, driver),
                                                 interrupts_{make_shared<Map<uint8_t, BCode_p>>()} {
    }

  public:
    static ptr<Interrupt> singleton(const Pattern &pattern,
                                    const ptr<PIN_DRIVER> &driver = nullptr) {
      static ptr<Interrupt> inter = ptr<Interrupt>(new Interrupt(pattern, driver));
      return inter;
    }
  };
} // namespace fhatos
#endif
