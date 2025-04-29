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
#ifdef NATIVE
#include <unistd.h>
#include <iostream>
#include <fstream>
#endif

namespace fhatos {
  static ID_p MEMORY_FURI = id_p("/sys/memory_t");

  class Memory final : public Uri {
  public:
    explicit Memory(const ID &id) : Uri(fURI("system_memory"), OType::URI, MEMORY_FURI, id_p(id)) {
    }


    // TODO: flash/partition/0x4434
    static ptr<Memory> singleton(const ID &id = ID("/sys/mem")) {
      static auto mem_p = ptr<Memory>(new Memory(id));
      return mem_p;
    }

#ifdef ESP_PLATFORM
    
static const TaskSnapshot_t *find_snapshot_linked_to_status(const TaskStatus_t *taskStatus,
  const TaskSnapshot_t *taskSnapshotArray,
  size_t taskSnapshotArraySize) {
for (size_t i = 0; i < taskSnapshotArraySize; ++i) {
if (*(int*)taskSnapshotArray[i].pxTCB == *(int*)taskStatus->xHandle) {
return &taskSnapshotArray[i];
}
}

return NULL;
}

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

    static Rec_p cpu_frequency() {
      return Obj::to_rec({{"freq", jnt(ESP.getCpuFreqMHz())}});
    }



#else

    static void process_mem_usage(double &vm_usage, double &resident_set) {
      using std::ios_base;
      using std::ifstream;
      using std::string;

      vm_usage = 0.0;
      resident_set = 0.0;

      ifstream stat_stream("/proc/self/stat", ios_base::in);

      string pid, comm, state, ppid, pgrp, session, tty_nr;
      string tpgid, flags, minflt, cminflt, majflt, cmajflt;
      string utime, stime, cutime, cstime, priority, nice;
      string O, itrealvalue, starttime;

      unsigned long vsize;
      long rss;

      stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
          >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
          >> utime >> stime >> cutime >> cstime >> priority >> nice
          >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

      stat_stream.close();

      long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
      vm_usage = vsize / 1024.0;
      resident_set = rss * page_size_kb;
    }

    static Rec_p main_memory() {
      double vm, rss2;
      process_mem_usage(vm, rss2);
      return Obj::to_rec({{"total", jnt(vm)},
        {"free", jnt(vm - rss2)},
        {"used", real(100.0f * (1.0f - (static_cast<float>(vm) - static_cast<float>(rss2)) / static_cast<float>(vm)),
                      REAL_FURI)}});
    }
#endif


    static void log_memory_stats() {
      LOG_WRITE(INFO, Memory::singleton().get(),
                L("!b{}!! {}\n", "main", Memory::singleton()->main_memory()->toString()));
#ifdef ESP_PLATFORM
      LOG_WRITE(INFO, Memory::singleton().get(), L("!b{}!! {}\n", "inst", Memory::singleton()->instruction_memory()->toString()));
      LOG_WRITE(INFO, Memory::singleton().get(), L("!b{}!! {}\n", "psram", Memory::singleton()->psram_memory()->toString()));
#endif
    }

    static void *import() {
      Typer::singleton()->save_type(*MEMORY_FURI, __());
      InstBuilder::build(Memory::singleton()->vid->add_component("main"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return Memory::main_memory();
          })
          ->save();
#ifdef ESP_PLATFORM
      InstBuilder::build(Memory::singleton()->vid->add_component("freq"))
           ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
           ->inst_f([](const Obj_p &, const InstArgs &) {
             return Memory::cpu_frequency();
           })
           ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("inst"))
      ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
      ->inst_f([](const Obj_p &, const InstArgs &) {
        return Memory::instruction_memory();
      })
      ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("psram"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return Memory::psram_memory();
          })
          ->save();
      InstBuilder::build(Memory::singleton()->vid->add_component("hwm"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {0, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return Memory::high_water_mark();
          })
          ->save();
#endif
      return nullptr;
    }
  };
}
#endif
