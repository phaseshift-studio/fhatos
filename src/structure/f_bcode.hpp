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
#include <process/actor/actor.hpp>

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = FOS_DEFAULT_ROUTER>
  class fBcode final : public Actor<PROCESS, ROUTER> {
  public:
    const Rec *rec;
    Obj *LOOP_KEY = new Str("loop");
    Obj *SETUP_KEY = new Str("setup");

    fBcode(const ID &id, const Rec *rec) :
        Actor<PROCESS, ROUTER>(
            id,
            // setup
            [this](const Actor<PROCESS, ROUTER> *actor) {
              try {
                const auto bcode_ptr = ptr<Bytecode>(
                    new Bytecode(new List<Inst *>(*this->rec->template get<Bytecode>(SETUP_KEY)->value())));
                Processor<Obj>(bcode_ptr).forEach(
                    [](const Obj *obj) { LOG(DEBUG, "setup: %s\n", obj->toString().c_str()); });
              } catch (fError &error) {
                LOG_EXCEPTION(error);
              }
            },
            // loop

            [this](const Actor<PROCESS, ROUTER> *actor) {
              try {
                const auto bcode_ptr = ptr<Bytecode>(
                    new Bytecode(new List<Inst *>(*this->rec->template get<Bytecode>(this->LOOP_KEY)->value())));
                Processor<Obj>(bcode_ptr).forEach(
                    [](const Obj *obj) { LOG(DEBUG, "loop: %s\n", obj->toString().c_str()); });
              } catch (fError &error) {
                LOG_EXCEPTION(error);
              }
            }),
        rec(new Rec(*rec->value())) {}
  };
} // namespace fhatos
#endif
