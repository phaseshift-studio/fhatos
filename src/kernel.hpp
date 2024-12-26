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
      PRINTER = ansi;
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
      string fhatos = STR(FOS_NAME) "-" STR(FOS_VERSION);
      string machine_sub_os = STR(FOS_MACHINE_SUBOS);
      string machine_arch = STR(FOS_MACHINE_ARCH);
      string machine_model = STR(FOS_MACHINE_MODEL);
      StringHelper::lower_case(fhatos);
      StringHelper::lower_case(machine_sub_os);
      StringHelper::lower_case(machine_arch);
      StringHelper::lower_case(machine_model);
      printer<>()->printf(FOS_TAB_4 "!b%s !y> !b%s !y> !b%s!!\n",
                          fhatos.c_str(), machine_sub_os.c_str(), machine_arch.c_str());
      if (!machine_model.empty())
        printer<>()->printf(FOS_TAB_6 " !y[!b%s!y]!!\n", machine_model.c_str());
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
      printer<>()->printf(FOS_TAB_4 "!blast reset reason: !y{}!!\n",r.c_str());
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

    static ptr<Kernel> import(const void *) {
      return Kernel::build();
    }

    /*static ptr<Kernel> import(const fURI &) {
      return Kernel::build();
    }*/

    static ptr<Kernel> mount(const Structure_p &structure) {
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      router()->attach(structure);
      return Kernel::build();
    }

    static ptr<Kernel> install(const Obj_p &obj) {
      if (obj->vid()) {
        ROUTER_WRITE(obj->vid(), obj,RETAIN);
        LOG_KERNEL_OBJ(INFO, router(), "!b{}!! !yobj!! loaded\n", obj->vid()->toString().c_str());
      }
      return Kernel::build();
    }

    static ptr<Kernel> process(const Process_p &process) {
      scheduler()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      ROUTER_WRITE(process->vid(), process,RETAIN);
      scheduler()->spawn(process);
      return Kernel::build();
    }

    static ptr<Kernel> eval(const Runnable &runnable) {
      runnable();
      return Kernel::build();
    }

    static void done(const char *barrier, const Supplier<bool> &ret = nullptr) {
      Scheduler::singleton()->barrier(barrier, ret, FOS_TAB_3 "!mPress!! <!yenter!!> !mto access terminal!! !gI/O!!\n");
      printer()->printf("\n" FOS_TAB_8 "{} !mFhat!gOS!!\n\n", Ansi<>::silly_print("shutting down").c_str());
#ifdef ESP_ARCH
      esp_restart();
#else
      exit(EXIT_SUCCESS);
#endif
    }
  };
} // namespace fhatos

#endif