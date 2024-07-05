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
#ifndef fhatos_f_bcode_hpp
#define fhatos_f_bcode_hpp
#include <language/fluent.hpp>
#include <language/obj.hpp>
#include <process/actor/actor.hpp>

namespace fhatos {
  template<typename PROCESS = Thread>
  class fBcode final : public Actor<PROCESS> {
  public:
    std::atomic_bool *setupComplete = new std::atomic_bool(false);
    const ptr<Obj> rec;
    ptr<BCode> SETUP_BCODE;
    ptr<BCode> LOOP_BCODE;

    fBcode(const ID &id, const Rec_p &rec) :
        Actor<PROCESS>(
            id,
            // setup
            [this](const Actor<PROCESS> *actor) {
              try {
                LOG(DEBUG, "Executing setup() bcode: %s\n", SETUP_BCODE->toString().c_str());
                Fluent(SETUP_BCODE).forEach<Obj>([this](const Obj_p &obj) {
                  LOG(DEBUG, "%s setup: %s\n", this->id()->toString().c_str(), obj->toString().c_str());
                });
                LOG(DEBUG, "Completeing setup() bcode: %s\n", SETUP_BCODE->toString().c_str());
              } catch (std::exception &error) {
                LOG_EXCEPTION(error);
                this->stop();
              }
            },
            // loop
            [this](const Actor<PROCESS> *actor) {
              try {
                Fluent(LOOP_BCODE).forEach<Obj>([this](const Obj_p &obj) {
                  LOG(DEBUG, "%s loop: %s\n", this->id()->toString().c_str(), obj->toString().c_str());
                });
              } catch (std::exception &error) {
                LOG_EXCEPTION(error);
                this->stop();
              }
            }),
        rec(rec), SETUP_BCODE(rec->rec_get(u_p("setup"))), LOOP_BCODE(rec->rec_get(u_p("loop"))) {
      LOG(DEBUG, "bcode program created: %s\n", rec->toString().c_str());
    }
  };
} // namespace fhatos
#endif
