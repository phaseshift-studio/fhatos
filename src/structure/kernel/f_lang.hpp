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

#ifndef fhatos_f_lang_hpp
#define fhatos_f_lang_hpp


#include <fhatos.hpp>
#include <language/parser.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include FOS_PROCESS(fiber.hpp)

namespace fhatos {
  template<typename PROCESS = Fiber, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fLang : public Actor<PROCESS, ROUTER> {
  public:
    static fLang *singleton() {
      static fLang lang = fLang();
      return &lang;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->subscribe(this->id().extend("parse"), [this](const Message &message) {
        Parser p = Parser(message.source);
        ptr<Bytecode> bcode = p.parse(message.payload->toStr().toString().c_str());
        LOG_TASK(INFO, this, "%s\n", bcode->toString().c_str());
        this->publish(ID(message.source), BinaryObj<>::fromObj(new Str(bcode->toString())),RETAIN_MESSAGE);
      });
      this->onQuery(this->id().extend("types").query("?"), Map<string,Obj>{
                      {"?bool", Str("bool<=bool")},
                      {"?int", Str("int<=int")},
                      {"?str", Str("str<=str")},
                      {"?lst", Str("lst<=lst")}
                    });
    }

  protected:
    fLang(const ID &id = FOS_DEFAULT_ROUTER::mintID("kernel", "lang")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
