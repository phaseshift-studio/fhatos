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
//#include STR(process/ptype/HARDWARE/scheduler.hpp)
#include STR(structure/stype/fs/HARDWARE/fs.hpp)
#include "model/fos/sys/scheduler/thread/thread.hpp"
#include "model/fos/sys/scheduler/scheduler.hpp"
#include "lang/mmadt/parser.hpp"
#include "util/memory_helper.hpp"
#include "util/print_helper.hpp"
#include "boot_config_loader.hpp"
#include "model/fos/sys/memory/memory.hpp"

namespace fhatos {
  class Kernel {
  public:
    static ptr<Kernel> build() {
      static auto kernel_p = make_shared<Kernel>();
      return kernel_p;
    }

    static ptr<Kernel> with_log_level(const LOG_TYPE level) {
      LOG_LEVEL = level;
      return Kernel::build();
    }

    static ptr<Kernel> using_printer(const ptr<Ansi<>> &ansi) {
      Options::singleton()->printer<Ansi<>>(ansi);
      return Kernel::build();
    }

    static ptr<Kernel> with_ansi_color(const bool use_ansi) {
      Options::singleton()->printer<Ansi<>>()->ansi_switch(use_ansi);
      return Kernel::build();
    }

    static ptr<Kernel> display_splash(const char *splash) {
      printer<>()->print(splash);
      return Kernel::build();
    }

    static ptr<Kernel> display_note(const char *notes) {
      printer<>()->printf(FOS_TAB_6 "%s\n", notes);
      return Kernel::build();
    }

    static ptr<Kernel> display_architecture() {
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
      if(!machine_model.empty())
        printer<>()->printf(FOS_TAB_6 " !y[!b%s!y]!!\n", machine_model.c_str());
      return Kernel::build();
    }

    static ptr<Kernel> display_reset_reason() {
#ifdef ESP_PLATFORM
      const esp_reset_reason_t reason = esp_reset_reason();
      string r;
      switch(reason) {
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
      printer<>()->printf(FOS_TAB_4 "!blast reset reason: !y%s!!\n", r.c_str());
#endif
      return Kernel::build();
    }

    static ptr<Kernel> display_memory() {
      Memory::singleton()->log_memory_stats();
      return Kernel::build();
    }

    static ptr<Kernel> using_scheduler(const ptr<Scheduler> &scheduler) {
      return Kernel::build();
    }

    static ptr<Kernel> using_router(const ptr<Router> &router) {
      return Kernel::build();
    }

    static ptr<Kernel> import(const void *) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      // TODO: arg should take a tid
      // LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! !ytype!! imported\n", obj->vid->toString().c_str());
      return Kernel::build();
    }

    /*static ptr<Kernel> import(const fURI &) {
      return Kernel::build();
    }*/

    static ptr<Kernel> mount(const Structure_p &structure) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Router::singleton()->attach(structure);
      return Kernel::build();
    }

    static ptr<Kernel> install(const Obj_p &obj) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      if(obj->vid) {
        Router::singleton()->write(*obj->vid, obj,RETAIN);
        LOG_WRITE(INFO, Router::singleton().get(), L("!b{}!! !yobj!! loaded\n", obj->vid->toString()));
      }
      return Kernel::build();
    }

    static ptr<Kernel> process(const Obj_p &thread) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      __().inst(Scheduler::singleton()->vid->add_component("spawn"), __().block(thread)).compute();
      return Kernel::build();
    }

    static ptr<Kernel> eval(const Runnable &runnable) {
      runnable();
      return Kernel::build();
    }

    static ptr<Kernel> using_boot_config(const fURI &boot_config_loader = fURI(FOS_BOOT_CONFIG_HEADER_URI)) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      boot_config_obj_copy_len = 0;
      bool to_free_boot = false;
      const ID_p config_id = id_p(FOS_BOOT_CONFIG_VALUE_ID);
      Obj_p config_obj = Obj::to_noobj();
      // boot from header file, file system, or wifi
      if(!boot_config_loader.equals(fURI(FOS_BOOT_CONFIG_HEADER_URI))) {
        fhatos::FSx::load_boot_config(boot_config_loader);
        if(boot_config_obj_copy_len > 0) {
          LOG_WRITE(INFO, Router::singleton().get(),
                    L("!b{} !yboot config file!! loaded (size: {} bytes)\n",
                      boot_config_loader.toString(), boot_config_obj_copy_len));
          to_free_boot = true;
        }
      }
      if(0 == boot_config_obj_copy_len) {
        if(boot_config_obj_len > 0) {
          boot_config_obj_copy = boot_config_obj;
          boot_config_obj_copy_len = boot_config_obj_len;
          LOG_WRITE(INFO, Router::singleton().get(),
                    L("!b" FOS_BOOT_CONFIG_HEADER_URI " !yboot config header!! loaded (size: {} bytes)\n",
                      boot_config_obj_copy_len));
        }
      }
      if(boot_config_obj_copy && boot_config_obj_copy_len > 0) {
        MemoryHelper::use_custom_stack(
            InstBuilder::build("boot_helper")
            ->inst_f([](const Obj_p &, const InstArgs &args) {
              mmadt::Parser::load_boot_config();
              return Obj::to_noobj();
            })->create(), Obj::to_noobj(), FOS_BOOT_CONFIG_MEM_USAGE);
        config_obj = Router::singleton()->read(*config_id);
      }
      if(to_free_boot && boot_config_obj_copy && boot_config_obj_copy_len > 0) {
        free(boot_config_obj_copy);
        boot_config_obj_len = 0;
      }
      if(config_obj->is_noobj())
        throw fError("!yboot loader config!! !rnot found!! in flash nor header");
      /////
      string boot_str = PrintHelper::pretty_print_obj(config_obj, 1);
      StringHelper::prefix_each_line(FOS_TAB_1, &boot_str);
      LOG_WRITE(INFO, Router::singleton().get(), L("\n{}\n", boot_str));
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      return Kernel::build();
    }

    static ptr<Kernel> drop_config(const string &id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Router::singleton()->write(string(FOS_BOOT_CONFIG_VALUE_ID) + "/" + id, noobj());
      LOG_WRITE(INFO, Router::singleton().get(), L("!b{} !yboot config!! dropped\n", id));
      return Kernel::build();
    }

    static void done(const char *barrier, const Supplier<bool> &ret = nullptr) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Router::singleton()->write(string(FOS_BOOT_CONFIG_VALUE_ID), noobj());
      LOG_WRITE(INFO, Scheduler::singleton().get(), L("!mscheduler barrier start: <!y{}!m>!!\n", "main"));
      while(!ROUTER_READ(Scheduler::singleton()->vid->extend("halt"))->or_else(dool(false))->bool_value()) {
        Scheduler::singleton()->loop();
      }
      LOG_WRITE(INFO, Scheduler::singleton().get(), L("!mscheduler barrier end: <!g{}!m>!!\n", "main"));
      Scheduler::singleton()->stop();
      printer()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n", Ansi<>::silly_print("shutting down").c_str());
#ifdef ESP_PLATFORM
      esp_restart();
#else
      exit(EXIT_SUCCESS);
#endif
    }
  };
} // namespace fhatos

#endif
