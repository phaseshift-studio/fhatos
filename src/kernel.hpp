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
#include STR(process/ptype/HARDWARE/scheduler.hpp)
#include STR(structure/stype/fs/HARDWARE/fs.hpp)
#include "process/process.hpp"
#include "lang/mmadt/parser.hpp"
#include "util/memory_helper.hpp"
#include "util/print_helper.hpp"
#include "boot_config_loader.hpp"
#ifdef ESP_ARCH
#include STR(structure/stype/fs/HARDWARE/fs.hpp)
#endif

namespace fhatos {

  class Kernel {

  public:
    static ptr<Kernel> build() {
      static auto kernel_p = make_shared<Kernel>();
      return kernel_p;
    }

    static ptr<Kernel> with_bcode(const BCode_p &bcode) {
      LOG(INFO, "with_bcode: %s\n", bcode->toString().c_str());
      const Objs_p objs = BCODE_PROCESSOR(bcode)->to_objs();
      LOG(INFO, "after_bcode: %s\n", objs->toString().c_str());
      return Kernel::build();
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
      Options::singleton()->printer<Ansi<>>()->on(use_ansi);
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
      printer<>()->printf(FOS_TAB_4 "!blast reset reason: !y%s!!\n",r.c_str());
#endif
      return Kernel::build();
    }

    static ptr<Kernel> display_memory(const string &label, const Rec_p &info) {
      if(!info->is_noobj())
        LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! %s\n", label.c_str(), info->toString().c_str());
      return Kernel::build();
    }

    static ptr<Kernel> using_scheduler(const ptr<Scheduler> &scheduler) {
      return Kernel::build();
    }

    static ptr<Kernel> using_router(const ptr<Router> &router) {
      return Kernel::build();
    }

    static ptr<Kernel> import(const void *) {
      // TODO: arg should take a tid
      // LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! !ytype!! imported\n", obj->vid->toString().c_str());
      return Kernel::build();
    }

    /*static ptr<Kernel> import(const fURI &) {
      return Kernel::build();
    }*/

    static ptr<Kernel> mount(const Structure_p &structure) {
      Scheduler::singleton()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      Router::singleton()->attach(structure);
      return Kernel::build();
    }

    static ptr<Kernel> install(const Obj_p &obj) {
      Scheduler::singleton()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      if(obj->vid) {
        Router::singleton()->write(obj->vid, obj,RETAIN);
        LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! !yobj!! loaded\n", obj->vid->toString().c_str());
      }
      return Kernel::build();
    }

    static ptr<Kernel> process(const Process_p &process) {
      Scheduler::singleton()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      // ROUTER_WRITE(process->vid, process,RETAIN);
      Scheduler::singleton()->spawn(process);
      return Kernel::build();
    }

    static ptr<Kernel> eval(const Runnable &runnable) {
      runnable();
      return Kernel::build();
    }

    static ptr<Kernel> using_boot_config(const fURI &boot_config_loader = fURI(FOS_BOOT_CONFIG_HEADER_URI)) {
      Scheduler::singleton()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      boot_config_obj_copy_len = 0;
      bool to_free_boot = false;
      const ID_p config_id = id_p(FOS_BOOT_CONFIG_VALUE_ID);
      Obj_p config_obj = Obj::to_noobj();
      // boot from header file, file system, or wifi
      if(!boot_config_loader.equals(fURI(FOS_BOOT_CONFIG_HEADER_URI))) {
        fhatos::FSx::load_boot_config(boot_config_loader);
        if(boot_config_obj_copy_len > 0) {
          LOG_KERNEL_OBJ(INFO, Router::singleton(),
                         "!b" FOS_BOOT_CONFIG_FS_URI " !yboot config file!! loaded (size: %i bytes)\n",
                         boot_config_obj_copy_len);
          to_free_boot = true;
        }
      }
      if(0 == boot_config_obj_copy_len) {
        if(boot_config_obj_len > 0) {
          boot_config_obj_copy = boot_config_obj;
          boot_config_obj_copy_len = boot_config_obj_len;
          LOG_KERNEL_OBJ(INFO, Router::singleton(),
                         "!b" FOS_BOOT_CONFIG_HEADER_URI " !yboot config header!! loaded (size: %i bytes)\n",
                         boot_config_obj_copy_len);
        }
      }
      if(boot_config_obj_copy && boot_config_obj_copy_len > 0) {
         MemoryHelper::use_custom_stack(mmadt::Parser::load_boot_config,FOS_BOOT_CONFIG_MEM_USAGE);
         config_obj = Router::singleton()->read(config_id);
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
      LOG_KERNEL_OBJ(INFO, Router::singleton(), "\n%s\n", boot_str.c_str());
      return Kernel::build();
    }

    static ptr<Kernel> drop_config(const string &id) {
      Scheduler::singleton()->feed_local_watchdog(); // ensure watchdog doesn't fail during boot
      Router::singleton()->write(id_p((string(FOS_BOOT_CONFIG_VALUE_ID) + "/" + id).c_str()), noobj());
      LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s !yboot config!! dropped\n", id.c_str());
      return Kernel::build();
    }

    static void done(const char *barrier, const Supplier<bool> &ret = nullptr) {
      Router::singleton()->write(id_p(string(FOS_BOOT_CONFIG_VALUE_ID).c_str()), noobj());
      Scheduler::singleton()->barrier(barrier, ret, FOS_TAB_3 "!mpress!! <!yenter!!> !mto access terminal!! !gI/O!!\n");
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
