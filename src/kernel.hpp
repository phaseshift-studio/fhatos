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

#include "boot_config_loader.hpp"
#include "fhatos.hpp"
#include "lang/mmadt/parser.hpp"
#include "model/fos/fos_obj.hpp"
#include "model/fos/s/fs/fs.hpp"
#include "model/fos/sys/router/memory/memory.hpp"
#include "model/fos/sys/scheduler/scheduler.hpp"
#include "model/fos/sys/scheduler/thread/thread.hpp"
#include "model/module.hpp"
#include "util/print_helper.hpp"
#ifdef ESP_PLATFORM
#include <esp_chip_info.h>
#include <esp_freertos_hooks.h>
#include <esp_system.h>
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif


namespace fhatos {

  class Kernel {
  public:
    Obj_p boot_config = Obj::to_rec();
    chrono::steady_clock::time_point boot_start_time;

    Obj_p &boot() { return this->boot_config; }

    Kernel *with_log_level(const LOG_TYPE level) {
      LOG_LEVEL = level;
      return this;
    }

    Kernel *with_ansi_color(const bool use_ansi) {
      Ansi<>::singleton()->ansi_switch(use_ansi);
      return this;
    }

    Kernel *display_splash(const char *splash) {
      printer<>()->print(splash);
      return this;
    }

    Kernel *display_note(const char *note) {
      printer<>()->printf(FOS_TAB_6 "!r.!go!bO!! %s !bO!go!r.!!\n", note);
      return this;
    }

    Kernel *evaluate_setup() {
      BOOTING = false;
      LOG_WRITE(INFO, this->boot().get(), L("!yexiting !bkernel boot !ystate!!: !rstricter!! type checking !genabled!!\n"));
      LOG_WRITE(INFO, this->boot().get(),
                L("!yapplying !bsetup !yinst!!\n" FOS_TAB_12 "{}\n", Kernel::boot()->rec_get("setup")->toString()));
      const Inst_p setup_inst = mmADT::delift(Kernel::boot()->rec_get("setup"))->inst_bcode_obj();
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      std::holds_alternative<Obj_p>(setup_inst->inst_f())
          ? std::get<Obj_p>(setup_inst->inst_f())->apply(Obj::to_noobj())
          : (*std::get<Cpp_p>(setup_inst->inst_f()))(Obj::to_noobj(), Obj::to_inst_args());
      return this;
    }

    Kernel *display_architecture() {
      string fhatos = STR(FOS_NAME) "-" STR(FOS_VERSION);
      string machine_sub_os = STR(FOS_MACHINE_SUBOS);
      string machine_arch = STR(FOS_MACHINE_ARCH);
      string machine_model = STR(FOS_MACHINE_MODEL);
      StringHelper::lower_case(fhatos);
      StringHelper::lower_case(machine_sub_os);
      StringHelper::lower_case(machine_arch);
      StringHelper::lower_case(machine_model);
      printer<>()->printf(FOS_TAB_4 "!b%s !y> !b%s !y> !b%s!!\n", fhatos.c_str(), machine_sub_os.c_str(),
                          machine_arch.c_str());
      if(!machine_model.empty())
        printer<>()->printf(FOS_TAB_6 " !y[!b%s!y]!!\n", machine_model.c_str());
      return this;
    }

    Kernel *start_timer() {
      this->boot_start_time = chrono::steady_clock::now();
      return this;
    }

    Kernel *stop_timer() {
      const auto endTime = std::chrono::steady_clock::now();
      const auto duration = std::chrono::duration<double>(endTime - this->boot_start_time);
      LOG_WRITE(INFO, this->boot().get(), L("!yboot time!!: {} !gseconds!!\n", duration.count()));
      return this;
    }

    Kernel *display_reset_reason() {
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
      return this;
    }

    Kernel *display_memory() {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Memory::singleton()->log_memory_stats();
      return this;
    }

    Kernel *import_module(const Pattern &pattern) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Typer::singleton()->import_module(pattern);
      return this;
    }

    Kernel *register_module(const Pattern &pattern) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      for(auto it = REGISTERED_MODULES->begin(); it != REGISTERED_MODULES->end();) {
        if(it->first.matches(pattern)) {
          Typer::singleton()->register_module(it->first, it->second);
          it = REGISTERED_MODULES->erase(it);
        } else {
          ++it;
        }
      }
      return this;
    }

    Kernel *using_typer(const ID &type_config_id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      const Obj_p typer_obj = Kernel::boot()->rec_get(type_config_id);
      if(!typer_obj->is_rec())
        throw fError("!ytyper config!! !rmust be!! a !brec!!: %s", typer_obj->toString().c_str());
      const ptr<Typer> typer = Typer::singleton(*typer_obj->vid);
      typer->rec_merge(typer_obj->rec_value());
      typer->save();
      TYPER_ID = typer->vid;
      Typer::import();
      mmADT::register_module();
      fOS::register_module();
      for(const Pattern &module_pattern:
          Typer::singleton()->obj_get("config/import")->or_else(Obj::to_lst())->lst_value<Pattern>([](const Uri_p u) {
            return u->uri_value();
          })) {
        Kernel::register_module(module_pattern);
      }
      LOG_WRITE(INFO, typer.get(),
                L("!gtyper!! configured\n" FOS_TAB_8 FOS_TAB_4 "{}\n", PrintHelper::pretty_print_obj(typer, 4, false)));
      return this;
    }

    Kernel *using_scheduler(const ID &scheduler_config_id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      const Obj_p scheduler_obj = Kernel::boot()->rec_get(scheduler_config_id);
      if(!scheduler_obj->is_rec())
        throw fError("!yscheduler obj!! !rmust be!! a !brec!!: %s", scheduler_obj->toString().c_str());
      const ptr<Scheduler> scheduler = Scheduler::singleton(*scheduler_obj->vid);
      scheduler->rec_merge(scheduler_obj->rec_value());
      // scheduler->obj_set("config", scheduler_obj->rec_get("config"));
      scheduler->save();
      SCHEDULER_ID = scheduler->vid;
      Scheduler::import();
      LOG_WRITE(INFO, scheduler.get(),
                L("!gscheduler!! configured\n" FOS_TAB_8 FOS_TAB_4 "{}\n",
                  PrintHelper::pretty_print_obj(scheduler, 4, false)));
      return this;
    }

    Kernel *using_router(const ID &router_config_id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      const Obj_p router_obj = Kernel::boot()->rec_get(router_config_id);
      if(!router_obj->is_rec())
        throw fError("!yrouter obj!! !rmust be!! a !brec!!: %s", router_obj->toString().c_str());
      const ptr<Router> router = Router::singleton(*router_obj->vid);
      router->rec_merge(router_obj->rec_value());
      // router->obj_set("config", router_obj->rec_get("config"));
      router->save();
      ROUTER_ID = router->vid;
      Router::import();
      LOG_WRITE(
          INFO, router.get(),
          L("!grouter!! configured\n" FOS_TAB_8 FOS_TAB_4 "{}\n", PrintHelper::pretty_print_obj(router, 4, false)));
      return this;
    }

    Kernel *drop_config(const ID &id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Kernel::boot()->obj_set(id, Obj::to_noobj());
      LOG_WRITE(INFO, Kernel::boot().get(), L("!b{} !yboot config!! dropped\n", id.toString()));
      return this;
    }

    Kernel *import(const void *) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      // TODO: arg should take a tid
      // LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! !ytype!! imported\n", obj->vid->toString().c_str());
      return this;
    }

    Kernel *mount(const Structure_p &structure) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Router::singleton()->attach(structure);
      return this;
    }

    Kernel *install(const Obj_p &obj) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      if(obj->vid) {
        Router::singleton()->write(*obj->vid, obj, RETAIN);
        LOG_WRITE(INFO, Router::singleton().get(), L("!b{}!! !yobj!! loaded\n", obj->vid->toString()));
      }
      return this;
    }

    Kernel *process(const Obj_p &thread) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      Scheduler::singleton()->spawn_thread(thread);
      return this;
    }

    Kernel *eval(const Runnable &runnable) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      runnable();
      return this;
    }

    Kernel *using_boot_config(const Obj_p &boot_config) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      this->boot_config = boot_config;
      LOG_WRITE(INFO, Kernel::boot().get(), L("!yboot loader config!!:\n"));
      string boot_str = PrintHelper::pretty_print_obj(boot_config, 4);
      StringHelper::prefix_each_line(FOS_TAB_2, &boot_str);
      LOG_WRITE(INFO, Kernel::boot().get(), L("\n{}\n", boot_str));
      return this;
    }

    Kernel *using_info(const ID &info_config_id) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
#ifdef NATIVE
      const bool plat = true;
#elif defined(ESP_PLATFORM)
      const bool plat = false;
#endif
      if(const Obj_p info = Kernel::boot()->rec_get(info_config_id); info->is_rec()) {
        info->rec_merge(rmap({{"platform", plat ? vri("native") : vri("esp32")},
                              {"arch", vri(STR(FOS_MACHINE_ARCH))},
                              {"model", vri(STR(FOS_MACHINE_MODEL))},
                              {"subos", vri(STR(FOS_MACHINE_SUBOS))}}));
        info->save();
      }
      ///////////////////////////////////////////////////////////////////////////////
      return this;
    }

    Kernel *using_boot_config(const fURI &boot_config_loader = fURI(FOS_BOOT_CONFIG_HEADER_URI)) {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
#ifdef NATIVE
      const string boot_dir = fs::current_path().string();
#else
      const string boot_dir = "/";
#endif
      LOG_WRITE(INFO, Router::singleton().get(), L("!yboot working directory!!: !b{}!!\n", boot_dir));
      boot_config_obj_copy_len = 0;
      Obj_p config_obj = Obj::to_noobj();
      // boot from obj encoded in filesystem
      if(!boot_config_loader.equals(FOS_BOOT_CONFIG_HEADER_URI)) {
        config_obj = fhatos::FS::load_boot_config(boot_config_loader);
      }
      /// boot from binary encoded header file
      if(config_obj->is_noobj()) {
        if(boot_config_obj_len > 0) {
          boot_config_obj_copy = boot_config_obj;
          boot_config_obj_copy_len = boot_config_obj_len;
        }
        if(boot_config_obj_copy && boot_config_obj_copy_len > 0) {
          config_obj = Memory::singleton()->use_custom_stack(InstBuilder::build("boot_config_stack")
                                                                 ->inst_f([](const Obj_p &, const InstArgs &) {
                                                                   auto temp = string((char *) boot_config_obj_copy);
                                                                   StringHelper::trim(temp);
                                                                   const Obj_p ret =
                                                                       mmadt::Parser::singleton()->parse(temp.c_str());
                                                                   return ret;
                                                                 })
                                                                 ->create(),
                                                             Obj::to_noobj(), FOS_BOOT_CONFIG_MEM_USAGE);
          Router::singleton()->write(FOS_BOOT_CONFIG_VALUE_ID, config_obj, true);
          LOG_WRITE(INFO, Router::singleton().get(),
                    L("!b{} !yboot config header!! loaded !g[!msize!!: {} bytes!g]!!\n", FOS_BOOT_CONFIG_HEADER_URI,
                      boot_config_obj_copy_len));
        }
      }
      if(config_obj->is_noobj())
        throw fError("!yboot loader config!! !rnot found!! in file system nor header");
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      return Kernel::using_boot_config(config_obj);
    }

    void setup() {
      FEED_WATCHDOG(); // ensure watchdog doesn't fail during boot
      // Router::singleton()->write(string(FOS_BOOT_CONFIG_VALUE_ID), noobj());
      // LOG_WRITE(INFO, Router::singleton().get(), L("!b# !yboot config!! dropped\n"));
      LOG_WRITE(INFO, Scheduler::singleton().get(), L("!mscheduler <!y{}!m>-loop!! started\n", "main"));
      // booting complete, tighter type constraints enforced
      delete REGISTERED_MODULES;
      if(this->boot()->rec_get("boot/drop")->or_else_<bool>(true)) {
        LOG_WRITE(INFO, this->boot().get(), L("!ydropping !bboot config!y obj!!\n"));
        Router::singleton()->loop();
        ROUTER_WRITE("/mnt/boot", Obj::to_noobj(), true);
        Router::singleton()->loop();
        this->boot_config.reset();
      }
      this->display_memory();
      BOOTING = false;
    }

#ifdef ESP_PLATFORM
    /*  // idle processor code
      Supplier<bool> loop_hook = [](){
        if(!BOOTING)
          Kernel::loop();
        return true;
      };
      const esp_err_t er = esp_register_freertos_idle_hook_for_cpu(loop_hook, 0);
      if(er != ESP_OK) {
        LOG_WRITE(ERROR, Obj::to_noobj().get(), L("unable to hook idle task\n"));
      }*/
#endif

    void loop() const {
#ifdef NATIVE
      while(!Scheduler::singleton()->obj_get("halt")->or_else_(false)) {
#else
      if(!Scheduler::singleton()->obj_get("halt")->or_else_(false)) {
#endif
        Scheduler::singleton()->loop();
        FEED_WATCHDOG();
        Router::singleton()->loop();
      }
#ifdef NATIVE
      {
#else
      else {
#endif
        LOG_WRITE(INFO, Scheduler::singleton().get(), L("!mscheduler <!y{}!m>-loop!! ended\n", "main"));
        Scheduler::singleton()->stop();
        printer()->printf("\n" FOS_TAB_8 "%s\n\n", Ansi<>::silly_print("Fare thee well FhatOS...").c_str());
#ifdef ESP_PLATFORM
        esp_restart();
#else
        exit(EXIT_SUCCESS);
#endif
      }
    }
  };
} // namespace fhatos


#endif
