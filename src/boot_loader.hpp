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
#include <model/console.hpp>
#include <model/terminal.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)
#include FOS_MQTT(mqtt.hpp)
#include <structure/obj_structure.hpp>
#include <structure/stype/heap.hpp>
#include <util/common_objs.hpp>
///////////// COMMON MODELS /////////////
#include <model/driver/fdriver.hpp>
#include <model/sys.hpp>
// #include <model/driver/gpio/arduino_gpio_driver.hpp>
// #include <model/driver/i2c/arduino_i2c_driver.hpp>
// #include <model/pin/gpio.hpp>
// #include <model/pin/interrupt.hpp>
// #include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
// #include FOS_BLE(ble.hpp)
#include <model/soc/esp/wifi.hpp>
#include <model/soc/memory/esp32/memory.hpp>
// #include FOS_TIMER(timer.hpp)
//#include <structure/stype/redirect.hpp>
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
        // load_process_spawner(); // TODO: remove
        load_structure_attacher(); // TODO: remove
        const ptr<Kernel> kp = Kernel::build()
            ->using_printer(Ansi<>::singleton())
            ->with_ansi_color(args_parser->option("--ansi", "true") == "true")
            ->with_log_level(LOG_TYPES.to_enum(args_parser->option("--log", "INFO")));
        if (args_parser->option("--headers", "true") == "true") {
          kp->displaying_splash(args_parser->option("--splash", ANSI_ART).c_str())
              ->displaying_architecture()
              ->displaying_history()
              ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!");
        }
        ////////////////////////////////////////////////////////////
        return kp->using_scheduler(Scheduler::singleton("/sys/scheduler"))
            ->using_router(Router::singleton("/sys/router/#"))
            ////////////////////////////////////////////////////////////
            ->mount(Heap::create("/sys/#"))
            ->mount(Heap::create("+/#"))
            ->mount(Heap::create("/type/#"))
            ->mount(Heap::create("/dev/#"))

            ->install(CommonObjs::type("/type/"))
            ->install(CommonObjs::terminal("/dev/terminal"))
            ->install(CommonObjs::parser("/dev/parser"))
            ->install(mmadt::mmADT::singleton())
            ->model("/model/sys/")
#ifdef ESP_ARCH
            ->mount(
                Wifi::singleton("/soc/wifi/+", Wifi::Settings(args_parser->option("--wifi:connect", "true") == "true",
                                                              args_parser->option("--wifi:mdns", STR(FOS_MACHINE_NAME)),
                                                              args_parser->option("--wifi:ssid", STR(WIFI_SSID)),
                                                              args_parser->option("--wifi:password", STR(WIFI_PASS)))))
#endif
            //   ->mount(Mqtt::create("//+/#", Mqtt::Settings(args_parser->option("--mqtt:client", STR(FOS_MACHINE_NAME)),
            //                                              args_parser->option("--mqtt:broker", STR(FOS_MQTT_BROKER)))))

#ifdef NATIVE
            ->mount(
                Mqtt::create("//driver/#", Mqtt::Settings(args_parser->option("--mqtt:client", STR(FOS_MACHINE_NAME)),
                                                         args_parser->option("--mqtt:broker", STR(FOS_MQTT_BROKER)))))
             ->mount(Heap::create("/driver/#"))
            ->install(fDriver::gpio_furi("/driver/gpio/furi", id_p("//driver/gpio")))
#elif defined(ESP_ARCH)
            ->mount(
               Mqtt::create("//driver/#", Mqtt::Settings(args_parser->option("--mqtt:client", STR(FOS_MACHINE_NAME)),
                                                        args_parser->option("--mqtt:broker", STR(FOS_MQTT_BROKER)))))
        ->mount(Heap::create("/driver/#"))
            ->install(fDriver::gpio_pin("/driver/gpio/pin",id_p("//driver/gpio")))
            ->mount(Memory::singleton("/soc/memory/#"))
        //->structure(BLE::create("/io/bt/#"))
        /* ->install(Redirect::create(
             "/redirect/",
             Pair<Pattern_p, Pattern_p>{p_p("/driver/gpio/:digital_read"), p_p("//driver/gpio/:digital_read")},
             Pair<Pattern_p, Pattern_p>{p_p("//driver/gpio/:digital_write"), p_p("/driver/gpio/:digital_write")}))*/
#endif

            //  ->driver(ArduinoGPIODriver::create("//gpio/arduino/request", "//gpio/arduino/response"))
            //->driver(ArduinoI2CDriver::create("//i2c/arduino/request", "//i2c/arduino/response"))
            //->structure(FileSystem::create("/io/fs/#", args_parser->option("--fs:mount", FOS_FS_MOUNT)))
            ->mount(Heap::create("/console/#"))
            ->process(Console::create("/console", "/dev/terminal/",
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