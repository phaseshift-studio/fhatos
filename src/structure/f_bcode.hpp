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

#ifndef fhatos_f_bcode_hpp
#define fhatos_f_bcode_hpp
#include <language/obj.hpp>

namespace fhatos {
  class fBcode final : public Actor<> {
  public:
    Rec *rec;
    Str *loopKey = new Str("loop");
    Str *setupKey = new Str("setup");

    fBcode(const ID &id, Rec *rec): Actor(id), rec(rec) {
    }

    void setup() override {
      const ptr<Bytecode> bcode_ptr = ptr<Bytecode>(const_cast<Bytecode *>(this->rec->get<Bytecode>(this->setupKey)));
      Processor<Obj> *processor = new Processor<Obj>(bcode_ptr);
      processor->forEach([](const Obj *obj) {
        LOG(INFO, "setup: %s\n", obj->toString().c_str());
      });
      delete processor;
    }

    void loop() override {
      const ptr<Bytecode> bcode_ptr = ptr<Bytecode>(const_cast<Bytecode *>(this->rec->get<Bytecode>(this->loopKey)));
      Processor<Obj> *processor = new Processor<Obj>(bcode_ptr);
      processor->forEach([](const Obj *obj) {
        LOG(INFO, "loop: %s\n", obj->toString().c_str());
      });
      delete processor;
    }
  };
}
#endif
