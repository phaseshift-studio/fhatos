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
    std::atomic<bool> *setupComplete = new std::atomic<bool>(false);
    const ptr<Obj> rec;
    ptr<BCode> SETUP_BCODE;
    ptr<BCode> LOOP_BCODE;

    fBcode(const ID &id, const ptr<Rec> &rec) :
        Actor<PROCESS, ROUTER>(
            id,
            // setup
            [this](const Actor<PROCESS, ROUTER> *actor) {
              bool done = ((fBcode *) actor)->setupComplete->load();
              ((fBcode *) actor)->setupComplete->store(true);
              if (!done) {
                try {
                  Processor<Obj>(SETUP_BCODE).forEach([this](const Obj *obj) {
                    LOG(DEBUG, "%s setup: %s\n", this->id().toString().c_str(), obj->toString().c_str());
                  });
                } catch (fError &error) {
                  LOG_EXCEPTION(error);
                }
              } else {
                LOG_TASK(ERROR, this, "setup() already executed\n");
              }
            },
            // loop
            [this](const Actor<PROCESS, ROUTER> *actor) {
              try {
                Processor<Obj>(LOOP_BCODE).forEach([this](const Obj *obj) {
                  LOG(DEBUG, "%s loop: %s\n", this->id().toString().c_str(), obj->toString().c_str());
                });
              } catch (fError &error) {
                LOG_EXCEPTION(error);
              }
            }),
        rec(rec), SETUP_BCODE(rec->rec_get(share(Uri("setup")))),
        LOOP_BCODE(rec->rec_get(share(Uri("loop")))) {}
  };
} // namespace fhatos
#endif
