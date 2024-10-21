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

#define MEMORY_REC_STRING "[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]"

namespace fhatos {

  class Memory : public External {

  protected:
    explicit Memory(const Pattern &pattern = Pattern("/soc/memory/#")) : External(pattern) {
      const Obj_p percent_type_def = parse("is(and(gte(0.0),lte(100.0)))");
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), percent_type_def);
      this->read_functions_->insert(
          {id_p(this->pattern_->resolve("./inst")), [this](const fURI_p &) {
             return List<Pair<ID_p, Obj_p>>(
                 {{id_p(this->pattern_->resolve("./inst")),
                   parse(StringHelper::format(
                       MEMORY_REC_STRING, ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                       ESP.getSketchSize() == 0
                           ? 0.0f
                           : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace())))))))}});
           }});
      this->read_functions_->insert(
          {id_p(this->pattern_->resolve("./heap")), [this](const fURI_p &) {
             const float used = (float) ESP.getHeapSize() == 0
                                    ? 0.0f
                                    : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize()))));
             return List<Pair<ID_p, Obj_p>>(
                 {{id_p(this->pattern_->resolve("./heap")),
                   parse(StringHelper::format(MEMORY_REC_STRING, ESP.getHeapSize(), ESP.getFreeHeap(), used))}});
           }});
      this->read_functions_->insert(
          {{id_p(this->pattern_->resolve("./psram")), [this](const fURI_p &) {
              const float used =
                  (float) ESP.getPsramSize() == 0
                      ? 0.0f
                      : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize()))));
              return List<Pair<ID_p, Obj_p>>(
                  {{id_p(this->pattern_->resolve("./psram")),
                    parse(StringHelper::format(MEMORY_REC_STRING, ESP.getPsramSize(), ESP.getFreePsram(), used))}});
            }}});
      this->read_functions_->insert(
          {{id_p(this->pattern_->resolve("./hwm")), [this](const fURI_p &) {
              const uint16_t free = FOS_ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
              const float used = FOS_ESP_THREAD_STACK_SIZE == 0
                                     ? 0.0f
                                     : (100.0f * (1.0f - ((float) free) / ((float) FOS_ESP_THREAD_STACK_SIZE)));
              return List<Pair<ID_p, Obj_p>>(
                  {{id_p(this->pattern_->resolve("./hwm")),
                    parse(StringHelper::format("[total=>%i,min_free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                                               FOS_ESP_THREAD_STACK_SIZE, free, used))}});
            }}});
    }
    // TODO: flash/partition/0x4434


  public:
    static ptr<Memory> singleton(const Pattern &pattern = Pattern("/soc/memory/#")) {
      static ptr<Memory> memory = ptr<Memory>(new Memory(pattern));
      return memory;
    }
  };
} // namespace fhatos
#endif