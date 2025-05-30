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

#ifdef NATIVE
#include "../memory.hpp"

namespace fhatos {

  Rec_p Memory::main_memory() const {
    using std::ios_base;
    using std::ifstream;
    using std::string;

    ifstream stat_stream("/proc/self/stat", ios_base::in);

    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;

    unsigned long vm_usage;
    long resident_set;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vm_usage >> resident_set; // don't care about the rest

    stat_stream.close();

      return Obj::to_rec({{"total", jnt(vm_usage)},
        {"free", jnt(vm_usage - resident_set)},
        {"used", real(100.0f * (1.0f - (static_cast<float>(vm_usage) - static_cast<float>(resident_set)) / static_cast<float>(vm_usage)),
                      REAL_FURI)}});

    }

  Rec_p Memory::inst_memory() const {
    return Obj::to_rec();
        }
  Rec_p Memory::psram_memory()  const {
    return Obj::to_rec();
  }
  Rec_p Memory::cpu_frequency()  const {
    return Obj::to_rec();
  }
  Rec_p Memory::high_water_mark()  const {
    return Obj::to_rec();
  }

  Obj_p Memory::internal_use_custom_stack(const Inst_p &inst, const Obj_p &lhs, const int stack_size) {
    return std::holds_alternative<Obj_p>(inst->inst_f())
            ? std::get<Obj_p>(inst->inst_f())->apply(lhs)
            : (*std::get<Cpp_p>(inst->inst_f()))(lhs, inst->inst_args()->clone());
    }
}
#endif
