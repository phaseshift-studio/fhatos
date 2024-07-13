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

#ifndef fhatos_kernel_hpp
#define fhatos_kernel_hpp

#include <fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include FOS_PROCESS(process.hpp)
#include <structure/io/terminal.hpp>

#include <language/types.hpp>
namespace fhatos {
  class Kernel {
  public:
    static Kernel *build() {
      static Kernel *kernel = new Kernel();
      return kernel;
    }
    static Kernel *withSplash(const char *splash) {
      Terminal::printer<>()->print(splash);
      return Kernel::build();
    }
    static Kernel *withNote(const char *notes) {
      Terminal::printer<>()->printf(FOS_TAB_4 "%s\n", notes);
      return Kernel::build();
    }
    static Kernel *usingScheduler(const Scheduler *scheduler) { return Kernel::build(); }
    static Kernel *onBoot(const List<Process *> processes) {
      bool success = true;
      for (Process *process: processes) {
        success = success && Scheduler::singleton()->spawn(process);
      }
      return Kernel::build();
    }
    static Kernel *loadModules(const List<ID> &modules) {
      for (const ID &id: modules) {
        Types::singleton()->loadExt(id);
      }
      return Kernel::build();
    }
    static Kernel *defaultOutput(const ID &output) {
      Terminal::currentOut(share(output));
      return Kernel::build();
    }
    static void done(const char *barrier = "kernel_barrier") {
      delete Kernel::build();
      Scheduler::singleton()->barrier(barrier);
    }
  };
} // namespace fhatos

#endif
