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
#include STR(structure/stype/mqtt/HARDWARE/mqtt.hpp)
#include "structure/stype/heap.hpp"
#include "lang/processor/processor.hpp"
///////////// COMMON MODELS /////////////
#ifdef NATIVE
#include "model/text/text.hpp"
#endif

#include STR(model/soc/memory/HARDWARE/memory.hpp)
#include STR(structure/stype/fs/HARDWARE/fs.hpp)

//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#ifdef CONFIG_SPIRAM_USE
#include "util/esp32/psram_allocator.hpp"
#endif
#include "model/soc/esp/ota.hpp"
#include "model/soc/esp/wifi.hpp"
#include "model/soc/memory/esp32/memory.hpp"
#include "model/driver/pin/arduino_gpio.hpp"
#include "model/driver/pin/arduino_pwm.hpp"
#include "model/driver/pin/arduino_i2c.hpp"
#include "model/sensor/aht10/aht10.hpp"
#include "model/ui/oled/oled.hpp"
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT string(getenv("FHATOS_HOME")).append("/data").c_str()
#else
#define FOS_FS_MOUNT "/"
#endif

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
#ifdef ESP_ARCH
        args_parser->set_option("--boot:config","/boot/boot_config.obj");
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
            ->mount(Heap<>::create("/fos/#"))
            ->mount(Heap<>::create("/boot/#", id_p("/sys/structure/boot")))
            ->using_boot_config(args_parser->option_furi("--boot:config", fURI(FOS_BOOT_CONFIG_HEADER_URI)))
            ->import(Router::import())
            ->drop_config("router")
            ->import(Scheduler::import())
            ->drop_config("scheduler");
        ////////////////////////////////////////////////////////////
        ////////////////// USER IMPORT(S) //////////////////////////
        ////////////////////////////////////////////////////////////
        return kp
            ->import(Heap<>::import("/sys/structure/lib/heap"))
            ->import(Mqtt::import("/sys/structure/lib/mqtt"))
            ////////////////// USER STRUCTURE(S)
            ->display_note("!r.!go!bO !yloading !blanguage !yobjs!! !bO!go!r.!!")
            ->mount(Heap<>::create(MMADT_SCHEME "/#", id_p("/sys/structure/mmadt")))
            ->import(mmadt::mmADT::import())
            ->display_note("!r.!go!bO !yloading !bio !yobjs!! !bO!go!r.!!")
            ->mount(Heap<>::create("/io/#", id_p("/sys/structure/io")))
            //->install(rec()->at(id_p("/io/lib")))
            ->import(Log::import("/io/lib/log"))
            ->import(Console::import("/io/lib/console"))
#ifdef NATIVE
            ->import(Text::import("/io/lib/text"))
#endif
            ->install(Terminal::singleton("/io/terminal"))
            ->install(mmadt::Parser::singleton("/io/parser"))
            ->install(Log::create("/io/log", Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/log"))
                                  ->or_else(Obj::to_rec({
                                      {"INFO", lst({vri("#")})},
                                      {"ERROR", lst({vri("#")})},
                                      {"DEBUG", lst()},
                                      {"TRACE", lst()}}))))
            ->drop_config("log")
            ->mount(Heap<>::create("+/#", id_p("/sys/structure/cache")))
            ->import(FSx::import("/sys/structure/lib/fs"))
            ->mount(FSx::create("/disk/#", id_p("/sys/structure/disk"),
                                Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/fs"))))
            ->drop_config("fs")
#if defined(ESP_ARCH)
            ->import(ArduinoGPIO::import("/io/lib/gpio"))
            ->import(ArduinoPWM::import("/io/lib/pwm"))
            ->import(ArduinoI2C::import("/io/lib/i2c"))
            ->mount(Heap<>::create("/sensor/#",id_p("/sys/structure/sensor")))
            ->import(AHT10::import("/sensor/lib/aht10"))
            ->import(OLED::import("/sensor/lib/oled"))
            ->mount(make_shared<Wifi>("/soc/wifi/+",
                  Wifi::Settings(args_parser->option_bool("--wifi:connect",true),
                                                             args_parser->option_string("--wifi:mdns", STR(FOS_MACHINE_NAME)),
                                                             args_parser->option_string("--wifi:ssid", STR(WIFI_SSID)),
                                                             args_parser->option_string("--wifi:password", STR(WIFI_PASS)))))
             ->mount(Structure::create<Memory>("/soc/memory/#"))
             ->mount(Heap<>::create("/soc/ota/#",id_p("/sys/structure/ota")))
             ->process(OTA::singleton("/soc/ota",Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/ota"))))
             ->drop_config("ota")
             //->mount(HeapPSRAM::create("/psram/#"))
#endif
            ->mount(Structure::create<Mqtt>("//io/#", id_p("/sys/structure/mqtt"),
                                            Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/mqtt"))->or_else(
                                                Obj::to_rec({
                                                    {"broker",
                                                     vri(args_parser->option_string(
                                                         "--mqtt:broker", STR(FOS_MQTT_BROKER)))},
                                                    {"client",
                                                     vri(args_parser->option_string(
                                                         "--mqtt:client", STR(FOS_MACHINE_NAME)))}}))))
            ->drop_config("mqtt")
            ->process(Console::create("/io/console",
                                      Router::singleton()->read(id_p(FOS_BOOT_CONFIG_VALUE_ID "/console"))))
            ->drop_config("console")
            ->eval([args_parser] {
              Router::singleton()->write(id_p("/sys/structure/boot"), Obj::to_noobj()); // shutdown the boot partition
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
