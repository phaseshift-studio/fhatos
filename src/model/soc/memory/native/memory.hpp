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
#include "../../../../lang/type.hpp"
#include "../../../../structure/stype/computed.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>

namespace fhatos {
  class Memory : public Computed {
  protected:
    List<ID_p> MEMORY_IDS_;

    explicit Memory(const Pattern &pattern = "/soc/memory/#") : Computed(pattern, id_p(pattern.retract_pattern())),
                                                                MEMORY_IDS_{{
                                                                  id_p(pattern.resolve("./inst")),
                                                                  id_p(pattern.resolve("./heap")),
                                                                  id_p(pattern.resolve("./psram")),
                                                                  id_p(pattern.resolve("./hwm"))}} {
    }

    // TODO: flash/partition/0x4434


  public:
    static ptr<Memory> singleton(const Pattern &pattern = "/soc/memory/#") {
      static ptr<Memory> memory = ptr<Memory>(new Memory(pattern));
      return memory;
    }

    virtual void setup() override {
      //DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    }

    static Rec_p main_memory() {
      int t_size = 0;
      int resident = 0;
      int share = 0;
      ifstream buffer("/proc/self/statm");
      buffer >> t_size >> resident >> share;
      buffer.close();

      const long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
      const double rss = resident * page_size_kb;
      const double shared_mem = share * page_size_kb;
      const double private_mem = rss - shared_mem;
      return Obj::to_rec({{"total", jnt(rss)}});
    }

    static Rec_p instruction_memory() {
      return Obj::to_noobj();
    }

    static Rec_p psram_memory() {
      return Obj::to_noobj();
    }

    static Rec_p high_water_mark() {
      return Obj::to_noobj();
    }
  };
} // namespace fhatos
#endif
