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

#include "fhatos.hpp"
#include FOS_PROCESS(scheduler.hpp)
#include "language/exts.hpp"
#include "model/terminal.hpp"
#include "process/actor/actor.hpp"
#include "process/process.hpp"

#include "language/types.hpp"

namespace fhatos {
  class Kernel {
    int boot_success = 0;
    int boot_failure = 0;

  public:
    static ptr<Kernel> build() {
      static ptr<Kernel> kernel_p = make_shared<Kernel>();
      return kernel_p;
    }

    static ptr<Kernel> with_log_level(const LOG_TYPE level) {
      Options::singleton()->log_level(level);
      return Kernel::build();
    }

    static ptr<Kernel> using_printer(const ptr<Ansi<>> &ansi) {
      Options::singleton()->printer<Ansi<>>(ansi);
      return Kernel::build();
    }

    static ptr<Kernel> with_ansi_color(const bool use_ansi) {
      Options::singleton()->printer<Ansi<>>()->on(use_ansi);
      return Kernel::build();
    }

    static ptr<Kernel> displaying_splash(const char *splash) {
      printer<>()->print(splash);
      return Kernel::build();
    }

    static ptr<Kernel> displaying_notes(const char *notes) {
      printer<>()->printf(FOS_TAB_4 "%s\n", notes);
      return Kernel::build();
    }

    static ptr<Kernel> displaying_architecture() {
      string arch = "<undefined>";
#ifdef ESP32
       arch = "ESP32";
#elif defined(ESP8266)
      arch = "ESP8266";
#elif defined(LINUX)
     arch = "Linux";
#elif defined(APPLE)
     arch = "MacOSX";
#endif
      printer<>()->printf("                                       "
                          "!bRunning on !y%s!!\n", arch.c_str());
      return Kernel::build();
    }

    static ptr<Kernel> using_scheduler(const ptr<Scheduler> &scheduler) {
      Options::singleton()->scheduler<Scheduler>(scheduler);
      return Kernel::build();
    }

    static ptr<Kernel> using_router(const ptr<Router> &router) {
      Options::singleton()->router<Router>(router);
      return Kernel::build();
    }

    static ptr<Kernel> structure(const Structure_p &structure) {
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      router()->attach(structure);
      return Kernel::build();
    }

    static ptr<Kernel> process(const Process_p &process) {
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      scheduler()->spawn(process);
      return Kernel::build();
    }

    static ptr<Kernel> model(const List<ID> &models) {
      for (const ID &id: models) {
        for (const auto &[id2, type]: Exts::exts(id)) {
          Types::singleton()->save_type(id_p(id2), type);
        }
      }
      return Kernel::build();
    }

    static ptr<Kernel> eval(const Runnable &runnable) {
      runnable();
      return Kernel::build();
    }

    static void done(const char *barrier = "kernel_barrier", Supplier<bool> ret = nullptr) {
      Scheduler::singleton()->barrier(barrier, ret, FOS_TAB_3 "!mPress!! <!yenter!!> !mto access terminal!! !gI/O!!\n");
      printer()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n", Ansi<>::silly_print("shutting down").c_str());
      exit(EXIT_SUCCESS);
    }
  };
} // namespace fhatos

#endif
