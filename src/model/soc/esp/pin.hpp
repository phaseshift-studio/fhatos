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
    explicit Pin(const Pattern &pattern,
                 const Function<uint8_t, Obj_p> &readFunc,
                 const BiConsumer<uint8_t, Obj_p> &writeFunc) :
        External(pattern) {
           this->read_functions_->insert({
             //////////////////
             //// READ PIN ////
             //////////////////
             {furi_p(this->pattern()->resolve("./+")),
              [this, readFunc](const fURI_p &pin_furi) {
                List<Pair<ID_p, Obj_p>> list;
                if (StringHelper::is_integer(pin_furi->name())) {
                  const uint8_t i = stoi(pin_furi->name());
                  list.push_back({id_p(*pin_furi), readFunc(i)});
                } else {
                  for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                    list.push_back({id_p(this->pattern()->resolve(fURI(string("./") + to_string(i)))), readFunc(i)});
                  }
                }
                return list;
              }}});
           this->write_functions_->insert(
            {///////////////////
             //// WRITE PIN ////
             ///////////////////
             {furi_p(this->pattern()->resolve("./+")), [writeFunc](const fURI_p &pin_furi, const Obj_p &pin_value) {
                List<Pair<ID_p, Obj_p>> list;
                const uint8_t i = stoi(pin_furi->name());
                pinMode(i, OUTPUT);
                writeFunc(i, pin_value);
                return list;
              }}});
           }
  };
} // namespace fhatos
#endif