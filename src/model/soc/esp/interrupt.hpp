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
#include <language/processor/processor.hpp>
#include <model/soc/esp/pin.hpp>

using namespace fhatos;
void ARDUINO_ISR_ATTR isr(void *arg) {
  const BCode_p bcode = PtrHelper::no_delete<BCode>(static_cast<BCode *>(arg));
  process(bcode, noobj());
};

namespace fhatos {
  class Interrupt : public Pin {

  protected:
    Map<uint8_t, BCode_p> interrupts_{};
    explicit Interrupt(const Pattern &pattern = "/soc/interrupt/#") :
        Pin(
            pattern,
            [this](const uint8_t pin) -> BCode_p { return this->interrupts_.count(pin) ? this->interrupts_.at(pin) : noobj(); },
            [this](const uint8_t pin, const BCode_p &bcode) -> void {
              if (this->interrupts_.count(pin)) {
                this->interrupts_.erase(pin);
                detachInterrupt(pin);
                LOG_STRUCTURE(INFO, this, "!bpin %i!! !yinterrupt!! detached\n", pin);
              }
              if (!bcode->is_noobj()) {
                this->interrupts_.insert({pin, bcode->clone()});
                BCode *bclone = new BCode(*bcode);
                attachInterruptArg(pin, isr, (void *) bclone, RISING);
                LOG_STRUCTURE(INFO, this, "!bpin %i!! !yinterrupt!! attached\n", pin);
              }
            }) {}

  public:
    static ptr<Interrupt> singleton(const Pattern &pattern = "/soc/interrupt/#") {
      static ptr<Interrupt> inter = ptr<Interrupt>(new Interrupt(pattern));
      return inter;
    }
  };
} // namespace fhatos
#endif