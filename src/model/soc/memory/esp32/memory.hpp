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

#include "../../../../fhatos.hpp"
#include "../../../../structure/stype/computed.hpp"

namespace fhatos {
  // static constexpr char *MEMORY_REC_STRING = "[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]";
  // static constexpr char *MEMORY_REC_STRING_2 = "[total=>%i,min_free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]";

  const static ID MEMORY_FURI = ID("/sys/lib/memory");

  class Memory : public Computed {
  public:
    explicit Memory(const Pattern &pattern, const ID_p &value_id = nullptr, const Rec_p &config = Obj::to_rec()) :
      Computed(pattern, id_p(MEMORY_FURI), value_id) {
      const ID_p percent_id = REAL_FURI;
      /*const ID_p percent_id = id_p(MMADT_SCHEME "/%");
      InstBuilder::build(*percent_id)
          ->domain_range(REAL_FURI, REAL_FURI)
          ->itype_and_seed(IType::ONE_TO_ONE)
          ->inst_f(OBJ_PARSER("is(gte(0.0)).is(lte(100.0))"))
          ->save();*/
    const  fURI inst = this->pattern->resolve("./inst");
     const fURI heap = this->pattern->resolve("./heap");
      const fURI psram = this->pattern->resolve("./psram");
     const  fURI hwm = this->pattern->resolve("./hwm");
      ///////
      this->read_functions_->emplace(
      inst, [this, inst, percent_id](const fURI &) {
        return IdObjPairs(
            initializer_list<Pair<ID, Obj_p>>({make_pair<ID, Obj_p>(inst, instruction_memory())}));
      });
      this->read_functions_->emplace(
      heap, [this, heap, percent_id](const fURI &) {
        return IdObjPairs({make_pair<ID,Obj_p>(heap, main_memory())});
      });
#ifdef CONFIG_SPIRAM_USE
      this->read_functions_->insert(
        {psram, [this, psram, percent_id](const fURI &) {
          return IdObjPairs(initializer_list<Pair<ID, Obj_p>>({make_pair<ID,Obj_p>(psram, psram_memory())}));
        }});
#endif
      this->read_functions_->insert(
      {hwm, [this, hwm, percent_id](const fURI &) {
        return IdObjPairs({make_pair<ID, Obj_p>(ID(hwm), high_water_mark())});
      }});
    }

    // TODO: flash/partition/0x4434
    static ptr<Memory> singleton(const Pattern &pattern, const ID &id = ID("")) {
      static auto mem_p = ptr<Memory>(new Memory(pattern, id_p(id)));
      return mem_p;
    }

    //static const  Real_p percentage = REAL_FURI;;

    static Rec_p instruction_memory() {
      return Obj::to_rec({{"total", jnt(ESP.getSketchSize() + ESP.getFreeSketchSpace())},
                          {"free", jnt(ESP.getFreeSketchSpace())},
                          {"used", real(ESP.getSketchSize() == 0
                                          ? 0.0f
                                          : (100.0f * (1.0f - (static_cast<float>(ESP.getFreeSketchSpace()) /
                                                               static_cast<float>(
                                                                 ESP.getSketchSize() + ESP.getFreeSketchSpace())))),
                                        REAL_FURI)}});
    }

    static Rec_p main_memory() {
      return Obj::to_rec({{"total", jnt(ESP.getHeapSize())},
                          {"free", jnt(ESP.getFreeHeap())},
                          {"used", real(static_cast<float>(ESP.getHeapSize()) == 0
                                          ? 0.0f
                                          : (100.0f * (1.0f - (static_cast<float>(ESP.getFreeHeap()) / static_cast<
                                                                 float>(ESP.
                                                                 getHeapSize())))), REAL_FURI)}});
    }

    static Rec_p psram_memory() {
      return Obj::to_rec({{"total", jnt(ESP.getPsramSize())},
                          {"free", jnt(ESP.getFreePsram())},
                          {"used",
                           real(static_cast<float>(ESP.getPsramSize()) == 0
                                  ? 0.0f
                                  : (100.0f * (1.0f - (static_cast<float>(ESP.getFreePsram()) / static_cast<float>(ESP.
                                                         getPsramSize())))), REAL_FURI)}});
    }

    static Rec_p high_water_mark() {
      const int free = FOS_ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
      return Obj::to_rec({{"total", jnt(FOS_ESP_THREAD_STACK_SIZE)},
                          {"min_free", jnt(free)},
                          {"used", real(FOS_ESP_THREAD_STACK_SIZE == 0
                                          ? 0.0f
                                          : (100.0f * (1.0f - static_cast<float>(free) / static_cast<float>(
                                                         FOS_ESP_THREAD_STACK_SIZE))), REAL_FURI)}});
    }
  };
} // namespace fhatos
#endif
