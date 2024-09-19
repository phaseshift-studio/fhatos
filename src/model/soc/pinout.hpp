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
#ifndef fhatos_pinout_hpp
#define fhatos_pinout_hpp

#include <fhatos.hpp>
#include <structure/stype/external.hpp>


namespace fhatos {

  class Pinout : public External {

  protected:
    explicit Pinout(const Pattern &pattern = "/soc/pinout/#") : External(pattern) {
      // TODO: flash/partition/0x44343
    }

  public:
    static ptr<Pinout> singleton(const Pattern &pattern = "/soc/pinout/#") {
      static ptr<Pinout> pinout = ptr<Pinout>(new Pinout(pattern));
      return pinout;
    }

    virtual List<ID_p> existing_ids(const fURI &match) override {
      List<ID_p> ids;
      for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
        ID_p pin = id_p(const_cast<Pinout *>(this)->pattern()->resolve("./pin/").extend(to_string(i)));
        if (pin->matches(match))
          ids.push_back(pin);
      }
      return ids;
    }

    virtual void setup() override {
      External::setup();

      this->read_functions_.insert(
          {share(this->pattern()->resolve("./pin/+")), [this](const fURI_p furi) {
             Map<ID_p, Obj_p> map;
             if (StringHelper::is_integer(furi->name())) {
               uint8_t pin_number = stoi(furi->name());
               map.insert({id_p(*furi), jnt(digitalRead(pin_number))});
             } else {
               for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                 map.insert(
                     {id_p(this->pattern()->resolve(fURI(string("./pin/") + to_string(i)))), jnt(digitalRead(i))});
               }
             }
             return map;
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded\n", this->pattern()->resolve("./pin/+").toString().c_str());
      this->write_functions_.insert(
          {share(this->pattern()->resolve("./pin/+")), [this](const fURI_p furi, const Obj_p &obj) {
             Map<ID_p, Obj_p> map;
             if (StringHelper::is_integer(furi->name())) {
               uint8_t pin_number = stoi(furi->name());
               digitalWrite(pin_number, obj->int_value());
               map.insert({id_p(*furi), obj});
             } else {
               for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                 map.insert(
                     {id_p(this->pattern()->resolve(fURI(string("./pin/") + to_string(i)))), jnt(digitalRead(i))});
               }
             }
             return map;
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !ywrite functions!! loaded\n", this->pattern()->resolve("./pin/+").toString().c_str());
    }
  };
} // namespace fhatos
#endif