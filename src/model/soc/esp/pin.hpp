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
#ifndef fhatos_pin_hpp
#define fhatos_pin_hpp

#include <fhatos.hpp>
#include <language/processor/processor.hpp>
#include <structure/stype/external.hpp>


namespace fhatos {

  class Pin : public External {

  protected:
    Map<ID_p, BCode_p, furi_p_less> interrupts_;
    explicit Pin(const Pattern &pattern, 
                 const Function<uint8_t, Int_p> &readFunc,
                 const BiConsumer<uint8_t, int> &writeFunc) :
        External(pattern) {

      //////////////////
      //// READ PIN ////
      //////////////////
      this->read_functions_.insert(
          {share(this->pattern()->resolve("./+")), [this, readFunc](const fURI_p &pin_furi) {
             List<Pair<ID_p, Obj_p>> list;
             if (StringHelper::is_integer(pin_furi->name())) {
               uint8_t i = stoi(pin_furi->name());
               list.push_back({id_p(*pin_furi), readFunc(i)});
             } else {
               for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                 list.push_back({id_p(this->pattern()->resolve(fURI(string("./") + to_string(i)))), readFunc(i)});
               }
             }
             return list;
           }});

      ///////////////////
      //// WRITE PIN ////
      ///////////////////
      this->write_functions_.insert(
          {furi_p(this->pattern()->resolve("./+")), [this, writeFunc](const fURI_p &pin_furi, const Int_p &pin_value) {
             List<Pair<ID_p, Obj_p>> list;
             uint8_t i = stoi(pin_furi->name());
             pinMode(i, OUTPUT);
             writeFunc(i, pin_value->int_value());
             return list;
           }});
    }
  };
} // namespace fhatos
#endif