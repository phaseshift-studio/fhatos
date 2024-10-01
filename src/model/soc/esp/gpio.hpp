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
#ifndef fhatos_gpio_hpp
#define fhatos_gpio_hpp

#include <fhatos.hpp>
#include <language/processor/processor.hpp>
#include <model/soc/esp/pin.hpp>
#include <structure/stype/external.hpp>


namespace fhatos {

  void ARDUINO_ISR_ATTR isr(void *arg) {
    const ID_p id = make_shared<ID>(static_cast<ID *>(arg)->toString().c_str());
    const BCode_p bcode = router()->read(id);
    process(bcode, uri(id));
  };

  class GPIO : public Pin {
  protected:
    Map<ID_p, BCode_p, furi_p_less> interrupts_;
    explicit GPIO(const Pattern &pattern = "/soc/gpio/#") :
        Pin(
            pattern,
            [](const uint8_t pin) -> Int_p {
              pinMode(pin, INPUT);
              return jnt(digitalRead(pin));
            },
            [](const uint8_t pin, const int value) {
              pinMode(pin, OUTPUT);
              digitalWrite(pin, value);
            }) {}

  public:
    static ptr<GPIO> singleton(const Pattern &pattern = "/soc/gpio/#") {
      static ptr<GPIO> gpio = ptr<GPIO>(new GPIO(pattern));
      return gpio;
    }

    virtual void setup() override {
      External::setup();
      ////////////////////////////////////////////////////////////
      ////////////////////////////// READ INTERRUPTS /////////////
      ////////////////////////////////////////////////////////////
      this->read_functions_.insert(
          {share(this->pattern()->resolve("./+/interrupt")), [this](const fURI_p furi) {
             List<Pair<ID_p, Obj_p>> list;
             uint8_t single = StringHelper::is_integer(furi->retract().name()) ? stoi(furi->retract().name()) : 99;
             for (uint8_t i = ((99 == single) ? 0 : single); i < ((99 == single) ? NUM_DIGITAL_PINS : (single + 1));
                  i++) {
               ID_p id = id_p(this->pattern()->resolve(fURI(string("./") + to_string(i) + "/interrupt")));
               LOG_STRUCTURE(DEBUG, this, "Searching interrupts for !b%s!!\n", id->toString().c_str());
               if (this->interrupts_.count(id)) {
                 LOG_STRUCTURE(DEBUG, this, "Interrupt !gfound!! for !b%s!!\n", id->toString().c_str());
                 list.push_back({id, this->interrupts_.at(id)});
               }
             }
             return list;
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded\n", this->pattern()->resolve("./#").toString().c_str());
      ////////////////////////////////////////////////////////////
      ////////////////////////////// WRITE INTERRUPTS ////////////
      ////////////////////////////////////////////////////////////
      this->write_functions_.insert(
          {furi_p(this->pattern()->resolve("./+/interrupt")), [this](const fURI_p furi, const Obj_p &obj) {
             uint8_t pin_number = stoi(furi->retract().name());
             if (this->interrupts_.count(id_p(*furi)))
               this->interrupts_.erase(id_p(*furi));
             if (obj->is_noobj()) {
               detachInterrupt(pin_number);
               LOG_STRUCTURE(INFO, this, "!bpin %i!! !yinterrupt!! detached\n", pin_number);
             } else {
               this->interrupts_.insert({id_p(*furi), obj});
               ID *heap_id = new ID(*furi);
               attachInterruptArg(pin_number, isr, (void *) heap_id, RISING);
               LOG_STRUCTURE(INFO, this, "!bpin %i!! !yinterrupt!! attached\n", pin_number);
             }
             return List<Pair<ID_p, Obj_p>>();
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !ywrite functions!! loaded\n",
                    this->pattern()->resolve("./+").toString().c_str());
    }
  };
} // namespace fhatos
#endif