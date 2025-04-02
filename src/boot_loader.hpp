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
#include "structure/router.hpp"
#include "util/argv_parser.hpp"
#include "lang/obj.hpp"
#include "lang/mmadt/mmadt_obj.hpp"
#include "lang/type.hpp"
#include "lang/mmadt/parser.hpp"
#include "model/fos/ui/terminal.hpp"
#include "model/fos/util/log.hpp"
#include "model/fos/sys/scheduler/thread/thread.hpp"
#include "structure/stype/heap.hpp"
#include "structure/stype/dsm.hpp"
#include "model/fos/sys/router/bus.hpp"
#include "structure/qtype/q_doc.hpp"
#include "structure/qtype/q_sub.hpp"
#include "structure/qtype/q_sub_mqtt.hpp"
#include "lang/processor/processor.hpp"
#include "model/fos/fos_obj.hpp"
/////////////////////////////////////////
///////////// COMMON MODELS /////////////
/////////////////////////////////////////

#include STR(structure/stype/fs/HARDWARE/fs.hpp)
#include "model/fos/util/text.hpp"
#ifdef NATIVE
#include STR(model/soc/memory/HARDWARE/memory.hpp)
//// FOS MODELS
#include "model/fos/io/gpio/gpio.hpp"
#include "model/fos/io/i2c/i2c.hpp"
#include "model/fos/ui/console.hpp"
#include "model/fos/sys/scheduler/scheduler.hpp"
////////////////////////////////////////
#elif defined(ESP_PLATFORM)
#include "model/fos/ui/rgbled/rgbled.hpp"
#include "model/fos/sensor/aht10/aht10.hpp"
#include "model/fos/io/pwm/pwm.hpp"
#include "model/fos/ui/oled/oled.hpp"
#include "model/fos/ui/rgbled/rgbled.hpp"
#include "model/fos/net/ota.hpp"
#include "model/fos/net/wifi.hpp"
#include "model/soc/memory/esp32/memory.hpp"
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
        if(args_parser->option_string("--boot:config", "NONE") == "NONE")
          args_parser->set_option("--boot:config", "boot/boot_config.obj");
        const ptr<Kernel> kp = Kernel::build()
            ->using_printer(Ansi<>::singleton())
            ->with_ansi_color(args_parser->option_bool("--ansi", true))
            ->with_log_level(LOG_TYPES.to_enum(args_parser->option_string("--log", "INFO")));
        if(args_parser->option_bool("--headers", true)) {
          kp->display_splash(args_parser->option_string("--splash", ANSI_ART).c_str())
              ->display_reset_reason()
              ->display_architecture()
              ->display_note("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!");
        }
        ////////////////////////////////////////////////////////////
        kp->display_note("!r.!go!bO !yloading !bsystem !yobjs!! !bO!go!r.!!")
            ->using_scheduler(Scheduler::singleton("/sys/scheduler"))
            ->using_router(Router::singleton("/sys/router"));
        if(args_parser->option_bool("--headers", true)) {
          kp->display_memory("inst memory", Memory::instruction_memory())
              ->display_memory("main memory", Memory::main_memory())
              ->display_memory("psram memory", Memory::psram_memory());
        }
        ////////////////////////////////////////////////////////////
        ////////////////// SYS STRUCTURE ///////////////////////////
        ///////////////////////////////////////////////////////////
        kp
            ->mount(Heap<>::create("/sys/#"))
            ->mount(Heap<>::create("/mnt/#"))
            ->mount(Heap<>::create("/boot/#", id_p("/mnt/boot")))
            ->using_boot_config(args_parser->option_furi("--boot:config", fURI(FOS_BOOT_CONFIG_HEADER_URI)))
            ->import(Router::import())
            ->drop_config("router")
            ->import(Scheduler::import())
            ->drop_config("scheduler");
        ////////////////////////////////////////////////////////////
        ////////////////// USER IMPORT(S) //////////////////////////
        ////////////////////////////////////////////////////////////
        return kp
            ->import(Heap<>::import("/mnt/lib/heap"))
            ->import(DSM::import("/mnt/lib/dsm"))
            ->import(Bus::import("/mnt/lib/bus"))
            ////////////////// USER STRUCTURE(S)
            ->mount(Heap<>::create(FOS_URI "/#", id_p("/mnt/fos")))
            ->import(fOS::import_types())
            ->import(fOS::import_q_procs())
            ->display_note("!r.!go!bO !yloading !bmmadt !ylang!! !bO!go!r.!!")
            ->mount(Structure::add_qproc(Heap<>::create(MMADT_SCHEME "/#", id_p("/mnt/mmadt")),
                                         QDoc::create("/mnt/mmadt/q/doc")))
            ->import(mmADT::import())
            ->import(mmADT::import_ext_types())
            ->eval([]() {
              Router::singleton()->write("/sys/vm/config",
                                         Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/vm"));
            })
            ->drop_config("vm")
            ////////
            ->display_note("!r.!go!bO !yloading !bfos !ymodels!! !bO!go!r.!!")
            ->import(fOS::import_io())
            ->import(fOS::import_sys())
            ->import(fOS::import_sensor())
            ->import(fOS::import_ui())
            ->import(fOS::import_util())
            /////////
            ->mount(Heap<>::create("/io/#", id_p("/mnt/io")))
            ->install(mmadt::Parser::singleton("/io/parser",
                                               Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/parser")))
            ->drop_config("parser")
            ->install(Log::create("/io/log",
                                  Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/log")
                                  ->or_else(Obj::to_rec({
                                      {"INFO", lst({vri("#")})},
                                      {"ERROR", lst({vri("#")})},
                                      {"WARN", lst()},
                                      {"DEBUG", lst()},
                                      {"TRACE", lst()}}))))
            ->drop_config("log")
            ->mount(Heap<>::create("+/#", id_p("/mnt/cache")))
            ->import(FSx::import("/sys/structure/lib/fs"))
            ->mount(FSx::create("/disk/#", id_p("/mnt/disk"),
                                Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/fs")))
            ->drop_config("fs")
#if defined(ESP_PLATFORM)
            ->mount(Heap<>::create("/sensor/#", id_p("/mnt/sensor")))
            ->display_note("!r.!go!bO !ycreating !bwifi !ymodel!! !bO!go!r.!!")
            ->install(*__(WIFIx::obj(Obj::to_rec({{"halt", dool(false)},
                                       {"config", __().from(FOS_BOOT_CONFIG_VALUE_ID "/wifi").compute().next()}}),
                                     "/io/wifi")).inst("connect").compute().begin())
            ->drop_config("wifi")
            ->mount(Structure::create<Memory>("/soc/memory/#"))
            /*->install(*__(OTA::obj({{"halt", dool(false)},
                                     {"config", __().from(FOS_BOOT_CONFIG_VALUE_ID "/ota").compute().next()}},
                                   "/io/ota")).inst("start").compute().begin())*/
            ->drop_config("ota")
            //->mount(HeapPSRAM::create("/psram/#"))
#endif
            ->mount(Structure::add_qproc(DSM::create("/shared/#", id_p("/mnt/dsm"),
                                                     Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/mqtt")->
                                                     or_else(
                                                         Obj::to_rec({
                                                             {"async", dool(true)},
                                                             {"broker",
                                                              vri(args_parser->option_string(
                                                                  "--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                                                             {"client",
                                                              vri(args_parser->option_string(
                                                                  "--mqtt:client", STR(FOS_MACHINE_NAME)))}}))),
                                         QSubMqtt::create(Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/mqtt")->
                                                          or_else(
                                                              Obj::to_rec({
                                                                  {"async", dool(true)},
                                                                  {"broker",
                                                                   vri(args_parser->option_string(
                                                                       "--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                                                                  {"client",
                                                                   vri(args_parser->option_string(
                                                                       "--mqtt:client", STR(FOS_MACHINE_NAME)))}})),
                                                          id_p("/mnt/dsm/"))))
            ->drop_config("mqtt")
            ->mount(
                Bus::create("/bus/#", id_p("/mnt/bus"), rec({{"source", vri("/bus")}, {"target", vri("//io")}})))
            ->install(Console::create("/io/console",
                                      Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/console")))
            ->drop_config("console")
            ->eval([args_parser] {
              // Router::singleton()->write("/mnt/boot", Obj::to_noobj()); // shutdown the boot partition
              //Router::singleton()->loop();
              delete args_parser;
            });
      } catch(const std::exception &e) {
        LOG_WRITE(ERROR, Obj::to_noobj().get(),
                  L("[{}] !rcritical!! !mFhat!gOS!! !rerror!!: {}\n", Ansi<>::silly_print("shutting down"), e.what()));
        Ansi<>::singleton()->println("");
      }
      return Kernel::build();
    }
  };
} // namespace fhatos
#endif
