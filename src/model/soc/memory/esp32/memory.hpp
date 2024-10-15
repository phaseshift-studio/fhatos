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

    CONST_CHAR(MEMORY_REC_STRING, "[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]");
    // CONST_CHAR(PERCENT_TYPE_DEF, "");

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

    virtual void setup() override {
      External::setup();
      const Obj_p percent_type_def = OBJ_PARSER("is(and(gte(0.0),lte(100.0)))");
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), percent_type_def);
      this->read_functions_.insert(
          {MEMORY_IDS_.at(0), [this](const fURI_p furi) {
             return List<Pair<ID_p, Obj_p>>(
                 {{MEMORY_IDS_.at(0),
                   parse(MEMORY_REC_STRING, ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                         ESP.getSketchSize() == 0
                             ? 0.0f
                             : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                  ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace()))))))}});
           }});
      this->read_functions_.insert(
          {MEMORY_IDS_.at(1), [this](const fURI_p furi) {
             return List<Pair<ID_p, Obj_p>>(
                 {{MEMORY_IDS_.at(1),
                   parse(MEMORY_REC_STRING, ESP.getHeapSize(), ESP.getFreeHeap(),
                         ESP.getHeapSize() == 0
                             ? 0.0f
                             : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize())))))}});
           }});
      this->read_functions_.insert(
          {{MEMORY_IDS_.at(2), [this](const fURI_p furi) {
              return List<Pair<ID_p, Obj_p>>(
                  {{MEMORY_IDS_.at(2),
                    parse(MEMORY_REC_STRING, ESP.getPsramSize(), ESP.getFreePsram(),
                          ESP.getPsramSize() == 0
                              ? 0.0f
                              : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize())))))}});
            }}});
      this->read_functions_.insert(
          {MEMORY_IDS_.at(3), [this](const fURI_p furi) {
             uint16_t free = ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
             float used = ESP_THREAD_STACK_SIZE == 0
                              ? 0.0f
                              : (100.0f * (1.0f - ((float) free) / ((float) ESP_THREAD_STACK_SIZE)));
             return List<Pair<ID_p, Obj_p>>(
                 {{MEMORY_IDS_.at(3), parse("[total=>%i,min_free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                                            ESP_THREAD_STACK_SIZE, free, used)}});
           }});
    }
  };
} // namespace fhatos
#endif