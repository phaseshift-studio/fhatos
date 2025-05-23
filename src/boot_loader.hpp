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
#ifndef fhatos_boot_loader_hpp
#define fhatos_boot_loader_hpp

#include "fhatos.hpp"
#include "kernel.hpp"
#include "lang/mmadt/parser.hpp"
#include "lang/obj.hpp"
#include "lang/processor/processor.hpp"
#include "model/fos/fos_obj.hpp"
#include "model/fos/s/heap.hpp"
#include "model/fos/sys/router/router.hpp"
#include "util/argv_parser.hpp"
/////////////////////////////////////////
///////////// COMMON MODELS /////////////
/////////////////////////////////////////

#ifdef NATIVE
//// FOS MODELS
#include "model/fos/io/gpio/gpio.hpp"
#include "model/fos/io/i2c/i2c.hpp"
#include "model/fos/sys/scheduler/scheduler.hpp"
#include "model/fos/ui/console.hpp"
////////////////////////////////////////
#elif defined(ESP_PLATFORM)
#include "model/fos/io/pwm/pwm.hpp"
#include "model/fos/net/ota.hpp"
#include "model/fos/net/wifi.hpp"
#include "model/fos/sensor/aht10/aht10.hpp"
#include "model/fos/ui/oled/oled.hpp"
#include "model/fos/ui/rgbled/rgbled.hpp"
#ifdef CONFIG_SPIRAM_USE
#include "util/esp32/psram_allocator.hpp"
#endif
#endif
////////////////////////////////////////

namespace fhatos {
  class BootLoader {
    /////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
  public:
    static ptr<Kernel> primary_boot(ArgvParser *args_parser) {
      std::srand(std::time(nullptr));
      try {
#ifdef CONFIG_SPIRAM_USE
        heap_caps_malloc_extmem_enable(FOS_EXTERNAL_MEMORY_LIMIT);
        // LOG(psramInit() ? INFO : ERROR, "PSRAM initialization\n");
#endif
        const ptr<Kernel> kp = Kernel::build()
                                   ->start_timer()
                                   ->using_printer(Ansi<>::singleton())
                                   ->with_ansi_color(args_parser->option_bool("--ansi", true))
                                   ->with_log_level(LOG_TYPES.to_enum(args_parser->option_string("--log", "INFO")));
        if(args_parser->option_bool("--headers", true)) {
          kp->display_splash(args_parser->option_string("--splash", ANSI_ART).c_str())
              ->display_reset_reason()
              ->display_architecture();
        }
        kp->display_note("!ymounting !bkernel !ystructures!!")
            ->display_memory()
            ->mount(Heap<>::create("/mnt/#"))
            ->mount(Heap<>::create("/sys/#", id_p("/mnt/sys")))
            ->mount(Heap<>::create("/boot/#", id_p("/mnt/boot")))
            ->mount(Heap<>::create("/fos/#", id_p("/mnt/fos")))
            ->mount(Heap<>::create("/mmadt/#", id_p("/mnt/mmadt")))
            ->using_boot_config(args_parser->option_furi("--boot:config", "/boot/boot_config.obj"));
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        kp->display_note("!yconfiguring !bkernel !yobjs!!")
            ->display_memory()
            ->using_router("router")
            ->drop_config("router")
            ->using_typer("typer")
            ->drop_config("typer")
            ->using_scheduler("scheduler")
            ->drop_config("scheduler");
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        load_processor();
        kp->display_note("!yinstalling !bfos/mmadt !ymodules!!")
            ->display_memory()
            ->mount(Heap<>::create("/io/#", id_p("/mnt/io")))
            ->import_module("/mmadt/#")
            ->import_module("/fos/s/#") //  structures
            ->import_module("/fos/q") // query processors
            ->import_module("/fos/sys/#") //  sys
            ->import_module("/fos/ui"); //  user interface
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        kp->display_note("!yevaluating !bsetup !yinst!!")
            ->display_memory()
            ->evalulating_setup()
            ->display_memory()
            ->stop_timer();
        /*
                ////////////////////////////////////////////////////////////
                ////////////////// USER IMPORT(S) //////////////////////////
                ////////////////////////////////////////////////////////////
                return kp
                    ////////////////// USER STRUCTURE(S)
                    ->mount(Heap<>::create(FOS_URI "/#", id_p("/mnt/fos")))
                    ->import(fOS::import_sys())
                    ->import(fOS::import_structure())
                    ->import(fOS::import_q_proc())
                    ->import(Processor::import())
                    ->display_note("!r.!go!bO !yloading !bmmadt !ylang!! !bO!go!r.!!")
                    ->mount(Structure::add_qproc(Heap<>::create(MMADT_SCHEME "/#", id_p("/mnt/mmadt")),
                                                 QDoc::create("/mnt/mmadt/q/doc")))
                    ->import(mmADT::import())
                    ->import(mmADT::import_ext_types())
                    ->eval([]() {
                      Router::singleton()->write("/sys/vm/config", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID
        "/vm"));
                    })
                    ->drop_config("vm")
                    ////////
                    ->display_note("!r.!go!bO !yloading !bfos !ymodels!! !bO!go!r.!!")
                    ->import(fOS::import_io())
                    ->import(fOS::import_sensor())
                    ->import(fOS::import_ui())
                    ->import(fOS::import_util())
                    /////////
                    ->mount(Heap<>::create("/io/#", id_p("/mnt/io")))
                    ->install(
                        mmadt::Parser::singleton("/io/parser", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID
        "/parser")))
                    ->drop_config("parser")
                    ->install(Log::create("/io/log", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/log")))
                    ->drop_config("log")
                    ->mount(Heap<>::create("+/#", id_p("/mnt/cache")))
                    ->mount(FS::create("/fs/#", id_p("/mnt/fs"), Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID
        "/fs")))
                    ->drop_config("fs")
        #ifdef ESP_PLATFORM
                    ->mount(Heap<>::create("/sensor/#", id_p("/mnt/sensor")))
                    ->display_note("!r.!go!bO !ycreating !bwifi !ymodel!! !bO!go!r.!!")
                    ->install(
                        *__(WIFIx::obj(Obj::to_rec({{"halt", dool(false)},
                                                    {"config", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID
        "/wifi")}}),
                                       "/io/wifi"))
                             .inst("connect")
                             .compute()
                             .begin())
                    ->drop_config("wifi")
                    /*->install(*__(OTA::obj({{"halt", dool(false)},
                                             {"config", __().from(FOS_BOOT_CONFIG_VALUE_ID "/ota").compute().next()}},
                                           "/io/ota")).inst("start").compute().begin())*/
        // ->drop_config("ota")
        //->mount(HeapPSRAM::create("/psram/#"))
        // #endif
        /* ->mount(Structure::add_qproc(
             DSM::create(
                 "/shared/#", id_p("/mnt/dsm"),
                 Router::singleton()
                     ->read(FOS_BOOT_CONFIG_VALUE_ID "/mqtt")
                     ->or_else(Obj::to_rec(
                         {{"async", dool(true)},
                          {"broker", vri(args_parser->option_string("--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                          {"client", vri(args_parser->option_string("--mqtt:client", STR(FOS_MACHINE_NAME)))}}))),
             QSubMqtt::create(
                 Router::singleton()
                     ->read(FOS_BOOT_CONFIG_VALUE_ID "/mqtt")
                     ->or_else(Obj::to_rec(
                         {{"async", dool(true)},
                          {"broker", vri(args_parser->option_string("--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                          {"client", vri(args_parser->option_string("--mqtt:client", STR(FOS_MACHINE_NAME)))}})),
                 id_p("/mnt/dsm/"))))
         ->drop_config("mqtt")
         //->mount(
         //    Bus::create("/bus/#", id_p("/mnt/bus"), rec({{"source", vri("/bus")}, {"target", vri("//io")}})))
         ->process(Console::create("/io/console", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/console")))
         ->drop_config("console")
         ->display_memory()
         ->eval([args_parser] {
           // Router::singleton()->write("/mnt/boot", Obj::to_noobj()); // shutdown the boot partition
           // Router::singleton()->loop();
           delete args_parser;
         });*/
      } catch(const fError &e) {
        LOG_WRITE(ERROR, Obj::to_noobj().get(),
                  L("[{}] !rcritical!! !mFhat!gOS!! !rerror!!: {}\n", Ansi<>::silly_print("shutting down"), e.what()));
        Ansi<>::singleton()->println("");
      }
      return Kernel::build();
    }
  };
} // namespace fhatos
#endif
