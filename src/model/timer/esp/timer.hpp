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
#ifndef fhatos_timer_hpp
#define fhatos_timer_hpp

#include <fhatos.hpp>
#include <language/processor/processor.hpp>
#include <structure/stype/external.hpp>

#define NUM_TIMERS 4

namespace fhatos {

  class Timer : public External {

    hw_timer_t *TIMER_0 = nullptr;
    hw_timer_t *TIMER_1 = nullptr;
    hw_timer_t *TIMER_2 = nullptr;
    hw_timer_t *TIMER_3 = nullptr;

    void IRAM_ATTR TIMER_0_ISR() { Timer::GENERAL_TIMER_ISR(0); }
    void IRAM_ATTR TIMER_1_ISR() { Timer::GENERAL_TIMER_ISR(1); }
    void IRAM_ATTR TIMER_2_ISR() { Timer::GENERAL_TIMER_ISR(2); }
    void IRAM_ATTR TIMER_3_ISR() { Timer::GENERAL_TIMER_ISR(3); }


  protected:
    static void GENERAL_TIMER_ISR(const uint8_t timer_number) {
      if (Timer::singleton()->interrupts_->count(timer_number)) {
        const BCode_p bcode = Timer::singleton()->interrupts_->at(timer_number);
        Options::singleton()->processor<Obj, BCode, Obj>(noobj(), bcode);
      } else {
        LOG_STRUCTURE(ERROR, Timer::singleton(), "no interrupt code found for TIMER_%i_ISR", timer_number);
      }
    };

    Map_p<uint8_t, BCode_p> interrupts_;
    explicit Timer(const Pattern &pattern = "/soc/timer/#") :
        External(pattern), interrupts_(make_shared<Map<uint8_t, BCode_p>>()) {
      this->read_functions_->insert(
          {furi_p(this->pattern_->resolve("./+")), [this](const fURI_p timer_furi) -> ReadRawResult_p {
             ReadRawResult_p list = make_shared<ReadRawResult>();
             if (StringHelper::is_integer(timer_furi->name())) {
               const uint8_t timer_number = stoi(timer_furi->name());
               if (this->interrupts_->count(timer_number)) {
                 list->push_back({id_p(*timer_furi), this->interrupts_->at(timer_number)});
               }
             } else {
               for (uint8_t timer_number = 0; timer_number < NUM_TIMERS; timer_number++) {
                 if (this->interrupts_->count(timer_number))
                   list->push_back({id_p(this->pattern_->resolve(fURI(string("./") + to_string(timer_number)))),
                                   this->interrupts_->at(timer_number)});
               }
             }
             return list;
           }});
    };


  public:
    static ptr<Timer> singleton(const Pattern &pattern = "/soc/timer/#") {
      static ptr<Timer> timer = ptr<Timer>(new Timer(pattern));
      return timer;
    }
  };
} // namespace fhatos
#endif