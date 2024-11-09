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
#include <language/type.hpp>
#include <structure/stype/computed.hpp>

namespace fhatos {

  // static constexpr char *MEMORY_REC_STRING = "[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]";
  // static constexpr char *MEMORY_REC_STRING_2 = "[total=>%i,min_free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]";

  class Memory : public Computed {

  protected:
    explicit Memory(const ID& id, const Pattern &pattern) : Computed(id, pattern) {
      const Obj_p percent_type_def = bcode({Insts::is(
          Insts::x_and(Insts::gte(real(0.0)), Insts::lte(real(100.0))))}); // parse("is(and(gte(0.0),lte(100.0)))"); //
      Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), percent_type_def);
      const ID_p inst = id_p(this->pattern_->resolve("./inst"));
      const ID_p heap = id_p(this->pattern_->resolve("./heap"));
      const ID_p psram = id_p(this->pattern_->resolve("./psram"));
      const ID_p hwm = id_p(this->pattern_->resolve("./hwm"));
      const ID_p percent = id_p(FOS_TYPE_PREFIX "real/%");
      ///////
      this->read_functions_->insert(
          {inst, [this, inst, percent](const fURI_p &) {
             const Rec_p r =
                 rec({{vri("total"), jnt(ESP.getSketchSize() + ESP.getFreeSketchSpace())},
                      {vri("free"), jnt(ESP.getFreeSketchSpace())},
                      {vri("used"),
                       real(ESP.getSketchSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                     ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace()))))),
                            percent)}});
             return make_shared<List<Pair<ID_p, Obj_p>>>(initializer_list<Pair<ID_p, Obj_p>>({{inst, r}}));
           }});
      this->read_functions_->insert(
          {heap, [this, heap, percent](const fURI_p &) {
             const Rec_p r =
                 rec({{vri("total"), jnt(ESP.getHeapSize())},
                      {vri("free"), jnt(ESP.getFreeHeap())},
                      {vri("used"),
                       real((float) ESP.getHeapSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize())))),
                            percent)}});
             return make_shared<List<Pair<ID_p, Obj_p>>>(initializer_list<Pair<ID_p, Obj_p>>({{heap, r}}));
           }});
      this->read_functions_->insert(
          {psram, [this, psram, percent](const fURI_p &) {
             const Rec_p r =
                 rec({{vri("total"), jnt(ESP.getPsramSize())},
                      {vri("free"), jnt(ESP.getFreePsram())},
                      {vri("used"),
                       real((float) ESP.getPsramSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize())))),
                            percent)}});
             return make_shared<List<Pair<ID_p, Obj_p>>>(initializer_list<Pair<ID_p, Obj_p>>({{psram, r}}));
           }});

      this->read_functions_->insert(
          {hwm, [this, hwm, percent](const fURI_p &) {
             const int free = FOS_ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
             const Rec_p r =
                 rec({{vri("total"), jnt(FOS_ESP_THREAD_STACK_SIZE)},
                      {vri("min_free"), jnt(free)},
                      {vri("used"), real(FOS_ESP_THREAD_STACK_SIZE == 0
                                             ? 0.0f
                                             : (100.0f * (1.0f - ((float) free) / ((float) FOS_ESP_THREAD_STACK_SIZE))),
                                         percent)}});
             return make_shared<List<Pair<ID_p, Obj_p>>>(initializer_list<Pair<ID_p, Obj_p>>({{hwm, r}}));
           }});
    }
    // TODO: flash/partition/0x4434


  public:
    static ptr<Memory> singleton(const ID& id, const Pattern &pattern) {
      static ptr<Memory> mem_p = ptr<Memory>(new Memory(id, pattern));
      return mem_p;
    }
  };
} // namespace fhatos
#endif