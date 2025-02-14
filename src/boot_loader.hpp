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
#include STR(process/ptype/HARDWARE/scheduler.hpp)
#include "lang/mmadt/type.hpp"
#include "lang/type.hpp"
#include "lang/mmadt/parser.hpp"
#include "model/console.hpp"
#include "model/terminal.hpp"
#include "model/log.hpp"
#include "model/fos/sys/thread/thread.hpp"
#include STR(structure/stype/mqtt/HARDWARE/mqtt.hpp)
#include "structure/stype/heap.hpp"
#include "lang/processor/processor.hpp"
#include "model/fos/type.hpp"
/////////////////////////////////////////
///////////// COMMON MODELS /////////////
/////////////////////////////////////////

#include STR(structure/stype/fs/HARDWARE/fs.hpp)
#include "model/text/text.hpp"
#ifdef NATIVE
#include STR(model/soc/memory/HARDWARE/memory.hpp)
//// FOS MODELS
#include "model/fos/io/gpio/gpio.hpp"
#include "model/fos/io/i2c/i2c.hpp"
////////////////////////////////////////
#elif defined(ESP_ARCH)
#include "model/fos/ui/rgbled/rgbled.hpp"
#include "model/fos/sensor/aht10/aht10.hpp"
#include "model/fos/io/pwm/pwm.hpp"
#include "model/fos/ui/oled/oled.hpp"
#include "model/fos/ui/rgbled/rgbled.hpp"
#include "model/soc/esp/ota.hpp"
#include "model/soc/esp/wifi.hpp"
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
#ifdef ESP_ARCH
        args_parser->set_option("--boot:config","/boot/boot_config.obj");
#else
          args_parser->set_option("--boot:config", "../conf/boot_config.obj");
#endif
        load_processor(); // TODO: remove
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
            ->import(Router::import())->drop_config("router")
            ->import(Scheduler::import())->drop_config("scheduler");
        ////////////////////////////////////////////////////////////
        ////////////////// USER IMPORT(S) //////////////////////////
        ////////////////////////////////////////////////////////////
        return kp
            ->import(Heap<>::import("/mnt/lib/heap"))
            ->import(Mqtt::import("/mnt/lib/mqtt"))
            ////////////////// USER STRUCTURE(S)
            ->display_note("!r.!go!bO !yloading !blanguage !yinsts!! !bO!go!r.!!")
            ->mount(Heap<>::create(MMADT_SCHEME "/#", id_p("/mnt/mmadt")))
            ->import(mmADT::import())
            ->display_note("!r.!go!bO !yloading !bmodel !ytypes!! !bO!go!r.!!")
            ->mount(Heap<>::create(FOS_URI "/#", id_p("/mnt/fos")))
            ->import(fOS::import_types())
            ->import(fOS::import_io())
            ->import(fOS::import_sensor())
            ->import(fOS::import_ui())
            ->import(fOS::import_util())
            ->import(ThreadX::import())
            ->mount(Heap<>::create("/io/#", id_p("/mnt/io")))
            ->import(Log::import("/io/lib/log"))
            ->import(Console::import())
            ->import(Text::import())
            ->install(Terminal::singleton())
            ->install(mmadt::Parser::singleton("/io/parser"))
            ->install(Log::create("/io/log",
                                  Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/log")
                                  ->or_else(Obj::to_rec({
                                      {"INFO", lst({vri("#")})},
                                      {"ERROR", lst({vri("#")})},
                                      {"DEBUG", lst()},
                                      {"TRACE", lst()}}))))
            ->drop_config("log")
            ->mount(Heap<>::create("+/#", id_p("/mnt/cache")))
            ->import(FSx::import("/sys/structure/lib/fs"))
            ->mount(FSx::create("/disk/#", id_p("/mnt/disk"),
                                Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/fs")))
            ->drop_config("fs")

#if defined(ESP_ARCH)
            ->mount(Heap<>::create("/sensor/#",id_p("/mnt/sensor")))
            ->mount(make_shared<Wifi>("/soc/wifi/+",
                  Wifi::Settings(args_parser->option_bool("--wifi:connect",true),
                                                             args_parser->option_string("--wifi:mdns", STR(FOS_MACHINE_NAME)),
                                                             args_parser->option_string("--wifi:ssid", STR(WIFI_SSID)),
                                                             args_parser->option_string("--wifi:password", STR(WIFI_PASS)))))
             ->mount(Structure::create<Memory>("/soc/memory/#"))
             ->mount(Heap<>::create("/soc/ota/#",id_p("/sys/structure/ota")))
           //  ->process(OTA::singleton("/soc/ota",Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/ota"))))
             ->drop_config("ota")
             //->mount(HeapPSRAM::create("/psram/#"))
#endif
            /*->mount(Structure::create<Mqtt>("//io/#", id_p("/sys/structure/mqtt"),
                                            Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/mqtt"))->or_else(
                                                Obj::to_rec({
                                                    {"broker",
                                                     vri(args_parser->option_string(
                                                         "--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                                                    {"client",
                                                     vri(args_parser->option_string(
                                                         "--mqtt:client", STR(FOS_MACHINE_NAME)))}}))))*/
            ->drop_config("mqtt")
            ->process(Console::create("/io/console",
                                      Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/console")))
            ->drop_config("console")
            ->eval([args_parser] {

              Router::singleton()->write(id_p("/mnt/boot"), Obj::to_noobj()); // shutdown the boot partition
              Router::singleton()->loop();
              delete args_parser;
            });
      } catch(const std::exception &e) {
        LOG(ERROR, "[%s] !rcritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::silly_print("shutting down").c_str(),
            e.what());
        throw;
      }
    }
  };
} // namespace fhatos
#endif
