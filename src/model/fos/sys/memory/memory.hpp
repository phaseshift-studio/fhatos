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
#include "../../../../furi.hpp"
#include "../../../../lang/mmadt/mmadt.hpp"
#include "../../../../lang/mmadt/mmadt_obj.hpp"
#include "../../../../lang/obj.hpp"
#ifdef NATIVE
#include <fstream>
#include <iostream>
#include <unistd.h>
#endif

namespace fhatos {
  static ID_p MEMORY_FURI = id_p("/sys/memory");
  using namespace mmadt;
  class Memory final : public Rec {
  protected:
    Obj_p internal_use_custom_stack(const Inst_p &inst, const Obj_p &lhs, int stack_size);

  public:
    explicit Memory(const ID &id) : Rec(std::make_shared<RecMap<>>(), OType::REC, MEMORY_FURI, id_p(id)) {}

    static int get_stack_size(const Obj_p &source, const fURI &relative_uri, const int default_stack_size = 0) {
     /* try {
        if(const Obj_p stack_size = source->obj_get(relative_uri); stack_size->is_int())
          return stack_size->int_value();
        if(const Int_p def = ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size"));
         def->is_int() && def->int_value() > 0)
          return def->int_value();
      } catch(const std::exception &) {
        // do nothing
      }*/
      return default_stack_size;
    }

    // TODO: flash/partition/0x4434
    static ptr<Memory> singleton(const ID &id = ID("/sys/mem")) {
      static auto mem_p = std::make_shared<Memory>(id);
      return mem_p;
    }

    Obj_p use_custom_stack(const Inst_p &inst, const Obj_p &lhs, const int stack_size) {
      const Obj_p ret = this->internal_use_custom_stack(inst, lhs, stack_size);
      return ret;
    }

    [[nodiscard]] Rec_p main_memory() const;

    [[nodiscard]] Rec_p inst_memory() const;

    [[nodiscard]] Rec_p psram_memory() const;

    [[nodiscard]] Rec_p cpu_frequency() const;

    [[nodiscard]] Rec_p high_water_mark() const;

    /* static const TaskSnapshot_t *find_snapshot_linked_to_status(const TaskStatus_t *taskStatus,
                                                                 const TaskSnapshot_t *taskSnapshotArray,
                                                                 size_t taskSnapshotArraySize) {
       for(size_t i = 0; i < taskSnapshotArraySize; ++i) {
         if(*(int *) taskSnapshotArray[i].pxTCB == *(int *) taskStatus->xHandle) {
           return &taskSnapshotArray[i];
         }
       }

       return NULL;
     }*/

    static void log_memory_stats() {
      LOG_WRITE(INFO, Memory::singleton().get(),
                L("!b{}!! {}\n", "main", Memory::singleton()->main_memory()->toString()));
#ifdef ESP_PLATFORM
      LOG_WRITE(INFO, Memory::singleton().get(),
                L("!b{}!! {}\n", "inst", Memory::singleton()->inst_memory()->toString()));
      LOG_WRITE(INFO, Memory::singleton().get(),
                L("!b{}!! {}\n", "psram", Memory::singleton()->psram_memory()->toString()));
#endif
    }

    static void *import() {
      Typer::singleton()->save_type(*MEMORY_FURI, __());
      InstBuilder::build(Memory::singleton()->vid->add_component("main"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) { return Memory::singleton()->main_memory(); })
          ->save();
#ifdef ESP_PLATFORM
      InstBuilder::build(Memory::singleton()->vid->add_component("freq"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) { return Memory::singleton()->cpu_frequency(); })
          ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("inst"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) { return Memory::singleton()->inst_memory(); })
          ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("psram"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) { return Memory::singleton()->psram_memory(); })
          ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("hwm"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) { return Memory::singleton()->high_water_mark(); })
          ->save();
#endif
      return nullptr;
    }
  };
} // namespace fhatos
#endif
