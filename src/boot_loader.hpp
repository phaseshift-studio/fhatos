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

#ifndef fhatos_boot_loader_hpp
#define fhatos_boot_loader_hpp

#include <fhatos.hpp>
#include <kernel.hpp>
#include <structure/router.hpp>
#include <util/argv_parser.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <language/mmadt/type.hpp>
#include <language/type.hpp>
#include <language/mmadt/parser.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include <model/log.hpp>
//#include FOS_FILE_SYSTEM(fs.hpp)
#include FOS_MQTT(mqtt.hpp)
#include <structure/stype/heap.hpp>
///////////// COMMON MODELS /////////////
#include <model/driver/driver.hpp>
#include <model/driver/fhatos/core_driver.hpp>
//#include <model/driver/gpio/arduino_gpio_driver.hpp>
//#include <model/driver/i2c/arduino_i2c_master_driver.hpp>
// #include <model/pin/gpio.hpp>
// #include <model/pin/interrupt.hpp>
// #include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#include <util/esp/psram_allocator.hpp>
// #include FOS_BLE(ble.hpp)
#include <model/soc/esp/wifi.hpp>
#include <model/soc/memory/esp32/memory.hpp>
// #include FOS_TIMER(timer.hpp)
// #include <structure/stype/redirect.hpp>
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT string(getenv("FHATOS_HOME")).append("/data").c_str()
#define ALLOC
#else
#define FOS_FS_MOUNT "/"
#define ALLOC PSRAMAllocator<Pair<const ID_p, Obj_p>>
#endif

namespace fhatos {
  class BootLoader {
    /////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
  public:
    static ptr<Kernel> primary_boot(const ArgvParser *args_parser) {
      std::srand(std::time(nullptr));
      try {
#ifdef BOARD_HAS_PSRAM
        heap_caps_malloc_extmem_enable(FOS_EXTERNAL_MEMORY_LIMIT);
        // LOG(psramInit() ? INFO : ERROR, "PSRAM initialization\n");
#endif
        load_processor(); // TODO: remove
        const ptr<Kernel> kp = Kernel::build()
            ->using_printer(Ansi<>::singleton())
            ->with_ansi_color(args_parser->option_bool("--ansi", true))
            ->with_log_level(LOG_TYPES.to_enum(args_parser->option_string("--log", "INFO")));
        if(args_parser->option_bool("--headers", true)) {
          kp->displaying_splash(args_parser->option_string("--splash", ANSI_ART).c_str())
              ->displaying_architecture()
              ->displaying_history()
              ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!");
        }
        ////////////////////////////////////////////////////////////
        return kp
            ->displaying_notes("!r.!go!bO !yloading !bsystem !yobjs!! !bO!go!r.!!")
            ->using_scheduler(Scheduler::singleton("/sys/scheduler"))
            ->using_router(Router::singleton("/sys/router"))
            ////////////////// SYS STRUCTURE
            ->mount(Heap<ALLOC>::create("/sys/#"))
            ->import(Scheduler::import())
            ->import(Router::import())
            ////////////////// USER STRUCTURE(S)
            ->displaying_notes("!r.!go!bO !yloading !blanguage !yobjs!! !bO!go!r.!!")
            ->mount(Heap<>::create("/type/#"))
            ->mount(Heap<>::create(FOS_SCHEME "/#"))
            ->mount(Heap<>::create(MMADT_SCHEME "/#"))
            ->import(FhatOSCoreDriver::import())
            ->install(Typer::singleton(FOS_SCHEME "/type"))
            ->import(mmadt::mmADT::import())
            ->displaying_notes("!r.!go!bO !yloading !bio !yobjs!! !bO!go!r.!!")
            ->mount(Heap<>::create("/io/#"))
            ->import(Log::import("/io/lib/log"))
            ->import(Console::import("/io/lib/console"))
            ->install(Terminal::singleton("/io/terminal"))
            ->install(Log::create("/io/log"))
            ->install(mmadt::Parser::singleton("/io/parser"))
            ->mount(Heap<>::create("+/#", "_cache"))
#if defined(ESP_ARCH)
            ->mount(
                Wifi::singleton("/soc/wifi/+", Wifi::Settings(args_parser->option_bool("--wifi:connect",true),
                                                             args_parser->option_string("--wifi:mdns", STR(FOS_MACHINE_NAME)),
                                                             args_parser->option_string("--wifi:ssid", STR(WIFI_SSID)),
                                                             args_parser->option_string("--wifi:password", STR(WIFI_PASS)))))
            // ->mount(HeapPSRAM::create("/psram/#"))
             ->mount(Memory::singleton("/soc/memory/#"))
            //->structure(BLE::create("/io/bt/#"))
#endif
            ->mount(Mqtt::create("//io/#",
                                 Mqtt::Settings(args_parser->option_string("--mqtt:client", STR(FOS_MACHINE_NAME)),
                                                args_parser->option_string("--mqtt:broker", STR(FOS_MQTT_BROKER))),
                                 "/io/mqtt"))
#if defined(NATIVE)
            //  ->install(ArduinoGPIODriver::load_remote("/driver/gpio/furi", id_p("//driver/gpio")))
            //   ->install(ArduinoI2CDriver::load_remote("/io/lib/", "i2c/master/furi", "//io/i2c"))
#endif
#if defined(ARDUINO) || defined(RASPBERRYPI)
            // ->install(ArduinoGPIODriver::load_local("/driver/gpio/pin", id_p("//driver/gpio")))
            // ->install(ArduinoI2CDriver::load_local("/driver/i2c/master/pin", id_p("//driver/i2c/master")))
#endif
            //->structure(FileSystem::create("/io/fs/#", args_parser->option("--fs:mount", FOS_FS_MOUNT)))


            ->mount(Heap<ALLOC>::create("/console/#"))
            ->process(Console::create("/console", Obj::to_rec({
                                        {"terminal",
                                          Obj::to_rec({
                                            {"stdout", vri("/io/terminal/:stdout")},
                                            {"stdin", vri("/io/terminal/:stdin")}})},
                                        {"nest", jnt(args_parser->option_int("--console:nest", 2))},
                                        {"prompt",
                                          str(args_parser->option_string("--console:prompt", "!mfhatos!g>!! "))},
                                        {"strict", dool(args_parser->option_bool("--console:strict", false))},
                                        {"log", vri(args_parser->option_string("--log", "INFO"))}
                                      })))
            ->eval([args_parser] { delete args_parser; });
      } catch(const std::exception &e) {
        LOG(ERROR, "[%s] !rcritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::silly_print("shutting down").c_str(),
            e.what());
        throw;
      }
    }
  };
} // namespace fhatos
#endif
