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
    List<ID_p> MEMORY_IDS_;
    explicit Memory(const Pattern &pattern = "/soc/memory/#") :
        External(pattern), MEMORY_IDS_{{id_p(pattern.resolve("./inst")), id_p(pattern.resolve("./heap")),
                                        id_p(pattern.resolve("./psram")), id_p(pattern.resolve("./hwm"))}} {}
    // TODO: flash/partition/0x4434


  public:
    static ptr<Memory> singleton(const Pattern &pattern = "/soc/memory/#") {
      static ptr<Memory> memory = ptr<Memory>(new Memory(pattern));
      return memory;
    }

    virtual List<ID_p> existing_ids(const fURI &match) override { return MEMORY_IDS_; }

    virtual void setup() override {
      External::setup();
      // Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX
      // "rec/mem_stat"),parse("~[total=>int[_],free=>int[_],used=>" FOS_TYPE_PREFIX "real/%%[_]]"));
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), parse("is(gte(0.0)).is(lte(100.0))"));
      this->read_functions_.insert(
          {MEMORY_IDS_.at(0), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {MEMORY_IDS_.at(0),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                        ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                        ESP.getSketchSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                 ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace()))))))}};
           }});
      this->read_functions_.insert(
          {MEMORY_IDS_.at(1), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {MEMORY_IDS_.at(1),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getHeapSize(),
                        ESP.getFreeHeap(),
                        ESP.getHeapSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize())))))}};
           }});
      this->read_functions_.insert(
          {MEMORY_IDS_.at(2), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {MEMORY_IDS_.at(2),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getPsramSize(),
                        ESP.getFreePsram(),
                        ESP.getPsramSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize())))))}};
           }});
      this->read_functions_.insert(
          {MEMORY_IDS_.at(3), [this](const fURI_p furi) {
             uint16_t free = ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
             float used = ESP_THREAD_STACK_SIZE == 0 ? 0.0f : (100.0f * (1.0f - ((float) free) / ((float) ESP_THREAD_STACK_SIZE)));
             return Map<ID_p, Obj_p>{
                 {MEMORY_IDS_.at(3), parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                                           ESP_THREAD_STACK_SIZE, free, used)}};
           }});
      // LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded:!y\n\t%s\n\t%s\n\t%s!!\n",
      //     FOS_INST_MEMORY_FURI->toString().c_str(), FOS_HEAP_MEMORY_FURI->toString().c_str(),
      //     FOS_PSRAM_MEMORY_FURI->toString().c_str());
    }
  };
} // namespace fhatos
#endif