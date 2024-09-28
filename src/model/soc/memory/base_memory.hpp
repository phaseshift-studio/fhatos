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
#ifndef fhatos_base_memory_hpp
#define fhatos_base_memory_hpp

#include <fhatos.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include <structure/stype/external.hpp>


namespace fhatos {
  struct MemInfo {
    double total_mem = -1.0;
    double free_mem = -1.0;;
    double usage_mem = -1.0;;
    double heap = -1.0;;
    double inst = -1.0; ;
    double psram = -1.0;;
    double hwm = -1.0;;
  };

  class BaseMemory : public External {
    CONST_CHAR(MEMORY_REC_STRING, "[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]");
    CONST_CHAR(PERCENT_TYPE_DEF, "is(gte(0.0)).is(lte(100.0))");

  protected:
    List<ID_p> MEMORY_IDS_;

    explicit BaseMemory(const Pattern &pattern = "/soc/memory/#") : External(pattern), MEMORY_IDS_{{
                                                                      id_p(pattern.resolve("./inst")),
                                                                      id_p(pattern.resolve("./heap")),
                                                                      id_p(pattern.resolve("./psram")),
                                                                      id_p(pattern.resolve("./hwm"))}} {
    }

    // TODO: flash/partition/0x4434


  public:
    virtual MemInfo get_mem_info() = 0;

    virtual void setup() override {
      External::setup();
      Types<>::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), parse(PERCENT_TYPE_DEF));

      this->read_functions_.insert(
        {MEMORY_IDS_.at(0), [this](const fURI_p furi) {
          const MemInfo mem_info = this->get_mem_info();
          return List<Pair<ID_p, Obj_p>>(
            {{MEMORY_IDS_.at(0),
              parse(MEMORY_REC_STRING, mem_info.total_mem, mem_info.free_mem, mem_info.usage_mem)}});
        }});
      /* this->read_functions_.insert(
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
            }});*/
      // LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded:!y\n\t%s\n\t%s\n\t%s!!\n",
      //     FOS_INST_MEMORY_FURI->toString().c_str(), FOS_HEAP_MEMORY_FURI->toString().c_str(),
      //     FOS_PSRAM_MEMORY_FURI->toString().c_str());
    }
  };

  static void enable_esp32_memory() {
    Types<>::singleton()->save_type(id_p(FOS_TYPE_PREFIX "/structure/soc/esp32/memory"), parse("[uri[_]=>_]"));
  }
} // namespace fhatos
#endif
