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
#ifndef fhatos_memory_hpp
#define fhatos_memory_hpp

#include <fhatos.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include <structure/stype/external.hpp>


namespace fhatos {

  class Memory : public External {

  protected:
    ID_p FOS_INST_MEMORY_FURI;
    ID_p FOS_HEAP_MEMORY_FURI;
    ID_p FOS_PSRAM_MEMORY_FURI;
    explicit Memory(const Pattern &pattern = "/soc/memory/#") :
        External(pattern), FOS_INST_MEMORY_FURI{id_p(pattern.resolve("./inst"))},
        FOS_HEAP_MEMORY_FURI{id_p(pattern.resolve("./heap"))}, FOS_PSRAM_MEMORY_FURI{id_p(pattern.resolve("./psram"))} {

    }
    // TODO: flash/partition/0x4434


  public:
    static ptr<Memory> singleton(const Pattern &pattern = "/soc/memory/#") {
      static ptr<Memory> memory = ptr<Memory>(new Memory(pattern));
      return memory;
    }

    virtual List<ID_p> existing_ids(const fURI &match) override {
      List<ID_p> ids;
      if (FOS_INST_MEMORY_FURI->matches(match))
        ids.push_back(FOS_INST_MEMORY_FURI);
      if (FOS_HEAP_MEMORY_FURI->matches(match))
        ids.push_back(FOS_HEAP_MEMORY_FURI);
      if (FOS_PSRAM_MEMORY_FURI->matches(match))
        ids.push_back(FOS_PSRAM_MEMORY_FURI);
      return ids;
    }

    virtual void setup() override {
      External::setup();
      // Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX
      // "rec/mem_stat"),parse("~[total=>int[_],free=>int[_],used=>" FOS_TYPE_PREFIX "real/%%[_]]"));
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), parse("is(gte(0.0)).is(lte(100.0))"));
      this->read_functions_.insert(
          {FOS_INST_MEMORY_FURI, [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {FOS_INST_MEMORY_FURI,
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                        ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                        ESP.getSketchSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                 ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace()))))))}};
           }});
      this->read_functions_.insert(
          {FOS_HEAP_MEMORY_FURI, [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {FOS_HEAP_MEMORY_FURI,
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getHeapSize(),
                        ESP.getFreeHeap(),
                        ESP.getHeapSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize())))))}};
           }});
      this->read_functions_.insert(
          {FOS_PSRAM_MEMORY_FURI, [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {FOS_PSRAM_MEMORY_FURI,
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getPsramSize(),
                        ESP.getFreePsram(),
                        ESP.getPsramSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize())))))}};
           }});
      // LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded:!y\n\t%s\n\t%s\n\t%s!!\n",
      //     FOS_INST_MEMORY_FURI->toString().c_str(), FOS_HEAP_MEMORY_FURI->toString().c_str(),
      //     FOS_PSRAM_MEMORY_FURI->toString().c_str());
    }
  };
} // namespace fhatos
#endif