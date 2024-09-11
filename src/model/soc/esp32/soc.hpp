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
#ifndef fhatos_mem_hpp
#define fhatos_mem_hpp

#include <fhatos.hpp>
#include <language/types.hpp>
#include <language/parser.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/external.hpp>


namespace fhatos {

class SoC : public Actor<Coroutine,External> {

protected:
explicit SoC(const ID id = "/soc/") : Actor(id) {

//TODO: *pin/a0
//TODO: flash/partition/0x44343
}

enum MEM_TYPE { INST, HEAP, PSRAM};
void write_memory_stats(MEM_TYPE mem_type) {
  switch(mem_type) {
case INST: this->write(id_p(this->id_->extend("inst")),parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
  ESP.getSketchSize() + ESP.getFreeSketchSpace(),
  ESP.getFreeSketchSpace(),
  ESP.getSketchSize() == 0 ? 0.0f : (100.0f *(1.0f-(((float)ESP.getFreeSketchSpace()) / ((float)(ESP.getSketchSize() + ESP.getFreeSketchSpace())))))),this->id_,RETAIN_MESSAGE);
  break;
case HEAP: this->write(id_p(this->id_->extend("heap")),parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
  ESP.getHeapSize(),
  ESP.getFreeHeap(),
  ESP.getHeapSize() == 0 ? 0.0f : (100.0f *(1.0f-(((float)ESP.getFreeHeap()) / ((float)ESP.getHeapSize()))))),this->id_,RETAIN_MESSAGE);
  break;
case PSRAM : this->write(id_p(this->id_->extend("psram")),parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
  ESP.getPsramSize(),
  ESP.getFreePsram(),
  ESP.getPsramSize() == 0 ? 0.0f : (100.0f *(1.0f-(((float)ESP.getFreePsram()) / ((float)ESP.getPsramSize()))))),this->id_,RETAIN_MESSAGE);
  break;
}
}

public:
static ptr<SoC> singleton(const ID id = "/soc/") {
    static ptr<SoC> soc = ptr<SoC>(new SoC(id));
    return soc;
}

//void publish_retained(const Subscription_p &subscription) override {
 // Actor::publish_retained(subscription);
//}

void setup() override {
    Actor::setup();
    Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"),parse("is(gte(0.0)).is(lte(100.0))"));
    Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/mem_stat"),parse("~[total=>int[_],free=>int[_],used=>" FOS_TYPE_PREFIX "real/%%[_]]"));
    this->write_memory_stats(INST);
    this->write_memory_stats(HEAP);
    this->write_memory_stats(PSRAM);
}   

 Obj_p read(const fURI_p &furi, const ID_p &source) override {
      this->write_memory_stats(furi->matches(this->id_->extend("inst/#")) ? INST : furi->matches(this->id_->extend("heap/#")) ? HEAP : PSRAM);
      return Actor::read(furi,source);
    }

    virtual void write(
      [[maybe_unused]] const ID_p &id, [[maybe_unused]] const Obj_p &obj,
      [[maybe_unused]] const ID_p &source, [[maybe_unused]] const bool retain) override {
        if(!source->equals(*this->id_)) {
            throw fError("only mem can update it's structure");
        } else {
            Actor::write(id,obj,source,retain);
        }
    }; 
  };
}
#endif