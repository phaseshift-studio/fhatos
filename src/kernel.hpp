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

#include <model/program.hpp>

#include <fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <language/exts.hpp>
#include <model/terminal.hpp>
#include <process/process.hpp>

#include <language/type.hpp>

namespace fhatos {
  class Kernel {
  public:
    static ptr<Kernel> build() {
      static auto kernel_p = make_shared<Kernel>();
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
                          "!brunning on !y%s!!\n",
                          arch.c_str());
      return Kernel::build();
    }

    static ptr<Kernel> displaying_history() {
#ifdef ESP_ARCH
      esp_reset_reason_t reason = esp_reset_reason();
      string r;
      switch (reason) {
        case ESP_RST_UNKNOWN:
          r = "unknown";
          break;
        case ESP_RST_POWERON:
          r = "powered up";
          break;
        case ESP_RST_EXT:
          r = "hardware reset";
          break;
        case ESP_RST_SW:
          r = "software reset";
          break;
        case ESP_RST_PANIC:
          r = "exception/panic";
          break;
        case ESP_RST_INT_WDT:
          r = "interrupt watchdog";
          break;
        case ESP_RST_TASK_WDT:
          r = "task watchdog";
          break;
        case ESP_RST_BROWNOUT:
          r = "power brownout";
          break;
        default:
          to_string(reason) + " code";
          break;
      }
      printer<>()->printf("                                       "
                          "!breset reason: !y%s!!\n",
                          r.c_str());
#endif
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
      //router()->write(process->id(), load_process(process));
      scheduler()->spawn(process);
      return Kernel::build();
    }

    static ptr<Kernel> program(const Structure_p &structure, const Process_p &process) {
      LOG(INFO, "!c[START]!!: !yloading program!!\n");
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      router()->attach(structure);
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      //router()->write(process->id(), load_process(process));
      scheduler()->spawn(process);
      LOG(INFO, "!c[ END ]!!: !yloading program!!\n");
      return Kernel::build();
    }

    static ptr<Kernel> model(const ID &model) {
      Exts::load_extension(model);
      return Kernel::build();
    }

    static ptr<Kernel> eval(const Runnable &runnable) {
      runnable();
      return Kernel::build();
    }

    static void done(const char *barrier, const Supplier<bool> &ret = nullptr) {
      Scheduler::singleton()->barrier(barrier, ret, FOS_TAB_3 "!mPress!! <!yenter!!> !mto access terminal!! !gI/O!!\n");
      Scheduler::singleton()->stop();
      printer()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n", Ansi<>::silly_print("shutting down").c_str());
#ifdef ESP_ARCH
      esp_restart();
#else
      exit(EXIT_SUCCESS);
#endif
    }
  };
} // namespace fhatos

#endif
