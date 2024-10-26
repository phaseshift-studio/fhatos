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
#include <language/type.hpp>
#include <language/mmadt/type.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)
#include FOS_MQTT(mqtt.hpp)
#include <process/obj_process.hpp>
#include <structure/obj_structure.hpp>
#include <structure/stype/heap.hpp>
///////////// COMMON MODELS /////////////
#include <model/driver/pin_driver.hpp>
#include <model/pin/gpio.hpp>
#include <model/pin/interrupt.hpp>
#include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#include FOS_BLE(ble.hpp)
#include <model/soc/esp/wifi.hpp>
#include <model/soc/memory/esp32/memory.hpp>
#include <structure/stype/ble/esp/ble.hpp>
#include FOS_TIMER(timer.hpp)
#include <structure/stype/redirect.hpp>
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
    static ptr<Kernel> primary_boot(const ArgvParser *args_parser) {
      std::srand(std::time(nullptr));
      try {
#ifdef BOARD_HAS_PSRAM
        heap_caps_malloc_extmem_enable(FOS_EXTERNAL_MEMORY_LIMIT);
        // LOG(psramInit() ? INFO : ERROR, "PSRAM initialization\n");
#endif
        load_processor(); // TODO: remove
        load_process_spawner(); // TODO: remove
        load_structure_attacher(); // TODO: remove
        const ptr<Kernel> kp = Kernel::build()
                                   ->using_printer(Ansi<>::singleton())
                                   ->with_ansi_color(args_parser->option("--ansi", "true") == "true")
                                   ->with_log_level(LOG_TYPES.to_enum(args_parser->option("--log", "INFO")));
        if (args_parser->option("--headers", "true") == "true") {
          kp->displaying_splash(args_parser->option("--splash", ANSI_ART).c_str())
              ->displaying_architecture()
              ->displaying_history()
              ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!")
              ->displaying_notes("Use !b:help!! for !yconsole commands!!");
        }
        ////////////////////////////////////////////////////////////
        return kp->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
            ->using_router(Router::singleton("/sys/router/#"))
            ////////////////////////////////////////////////////////////
            ->structure(Heap::create("+/#"))
            //
            ->program(Heap::create("/type/#"),Type::singleton("/type/"))
            //
           ->eval([]() {
             mmadt::mmADT::load();
           })
            //
            ->structure(Terminal::singleton("/terminal/#"))
            //
            ->program(Heap::create("/parser/#"),Parser::singleton("/parser/"))
            //
            ->model("/model/sys/")
        //
#ifdef ESP_ARCH
            ->structure(
                Wifi::singleton("/soc/wifi/+", Wifi::Settings(args_parser->option("--wifi:connect", "true") == "true",
                                                              args_parser->option("--wifi:mdns", STR(FOS_MACHINE_NAME)),
                                                              args_parser->option("--wifi:ssid", STR(WIFI_SSID)),
                                                              args_parser->option("--wifi:password", STR(WIFI_PASS)))))
#endif
            ->structure(
                Mqtt::create("//+/#", Mqtt::Settings(args_parser->option("--mqtt:client", STR(FOS_MACHINE_NAME)),
                                                     args_parser->option("--mqtt:broker", STR(FOS_MQTT_BROKER)))))
#ifdef NATIVE
            ->structure(GPIO<fURIPinDriver>::create(
                "/soc/gpio/#", fURIPinDriver::create("//read/soc/gpio/#", "//write/soc/gpio/#", "//remote/soc/pwm/#")))
            ->structure(PWM<fURIPinDriver>::create(
                "/soc/pwm/#", fURIPinDriver::create("//read/soc/gpio/#", "//write/soc/gpio/#", "//remote/soc/pwm/#")))
            ->structure(Interrupt<fURIPinDriver>::singleton(
                "/soc/interrupt/#",
                fURIPinDriver::create("//read/soc/gpio/#", "//write/soc/gpio/#", "//remote/soc/pwm/#")))
#elif defined(ESP_ARCH)
            ->structure(GPIO<ArduinoPinDriver>::create("/soc/gpio/#", ArduinoPinDriver::singleton()))
            ->structure(PWM<ArduinoPinDriver>::create("/soc/pwm/#", ArduinoPinDriver::singleton()))
            ->structure(Interrupt<ArduinoPinDriver>::singleton("/soc/interrupt/#", ArduinoPinDriver::singleton()))
            ->structure(Timer::singleton("/soc/timer/#"))
            ->structure(Memory::singleton("/soc/memory/#"))
            ->structure(BLE::create("/io/bt/#"))
            ->process(Redirect::create("/redirect/",
                                       Pair<Pattern_p, Pattern_p>{p_p("/soc/gpio/#"), p_p("//read/soc/gpio/#")},
                                       Pair<Pattern_p, Pattern_p>{p_p("//write/soc/gpio/#"), p_p("/soc/gpio/#")}))
#endif
            ->structure(FileSystem::create("/io/fs/#", args_parser->option("--fs:mount", FOS_FS_MOUNT)))
            ->structure(Heap::create("/console/#"))
            ->process(Console::create("/console/", "/terminal/:owner",
                                      Console::Settings(args_parser->option("--console:nest", "false") == "true",
                                                        args_parser->option("--ansi", "true") == "true",
                                                        args_parser->option("--console:strict", "false") == "true",
                                                        LOG_TYPES.to_enum(args_parser->option("--log", "INFO")))))
            ->eval([args_parser] { delete args_parser; });
      } catch (const std::exception &e) {
        LOG(ERROR, "[%s] !rCritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::silly_print("shutting down").c_str(),
            e.what());
        throw;
      }
    }
  };
} // namespace fhatos
#endif
