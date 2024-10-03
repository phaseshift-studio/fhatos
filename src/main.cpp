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

#include <fhatos.hpp>
#include <kernel.hpp>
#include <structure/router.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <language/types.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)
#include FOS_MQTT(mqtt.hpp)
#include <model/fs/base_fs.hpp>
#include <process/obj_process.hpp>
#include <structure/obj_structure.hpp>
#include FOS_MEMORY(memory.hpp)

#ifdef ESP_ARCH
#include <model/soc/esp/gpio.hpp>
#include <model/soc/esp/interrupt.hpp>
#include <model/soc/esp/pwm.hpp>
#include <model/soc/esp/wifi.hpp>
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT "./build/tmp"
#else
#define FOS_FS_MOUNT "/"
#endif

namespace fhatos {
  class ArgvParser {
    Map<const string, string> map_ = Map<const string, string>();

  public:
    void init(const int &argc, char **argv) {
      for (int i = 1; i < argc; ++i) {
        const auto temp = string(argv[i]);
        size_t j = temp.find_first_of('=');
        if (j != string::npos) {
          string key = temp.substr(0, j);
          string value = temp.substr(j + 1);
          this->map_.insert({key, value});
        } else {
          string key = temp;
          string value = "";
          this->map_.insert({key, value});
        }
      }
    }

    string option(const string &option, const char *or_else) const {
      return this->map_.count(option) ? this->map_.at(option) : or_else;
    }
  };
} // namespace fhatos
using namespace fhatos;
ArgvParser *args_parser = new ArgvParser();
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void setup() {
  std::srand(std::time(nullptr));
  try {
#ifdef BOARD_HAS_PSRAM
    // LOG(psramInit() ? INFO : ERROR, "PSRAM initialization\n");
#endif
    load_processor(); // TODO: remove
    load_process_spawner(); // TODO: remove
    load_structure_attacher(); // TODO: remove
    Kernel::build()
        ->using_printer(Ansi<>::singleton())
        ->with_ansi_color(args_parser->option("--ansi", "true") == "true")
        ->with_log_level(LOG_TYPES.to_enum(args_parser->option("--log", "INFO")))
        ->displaying_splash(ANSI_ART)
        ->displaying_architecture()
        ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!")
        ->displaying_notes("Use !b:help!! for !yconsole commands!!")
        ////////////////////////////////////////////////////////////
        ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
        ->using_router(Router::singleton("/sys/router/#"))
        ////////////////////////////////////////////////////////////
        ->structure(KeyValue::create("+/#"))
        //
        ->structure(KeyValue::create("/type/#"))
        ->process(Types::singleton("/type/"))
        //
        ->structure(Terminal::singleton("/terminal/#"))
        //
        ->structure(KeyValue::create("/parser/#"))
        ->process(Parser::singleton("/parser/"))
#ifdef ESP_ARCH
        ->structure(GPIO::singleton("/soc/gpio/#"))
        ->structure(PWM::singleton("/soc/pwm/#"))
        ->structure(Memory::singleton("/soc/memory/#"))
        //->structure(Interrupt::singleton("/soc/interrupt/#"))
        ->structure(Wifi::singleton("/soc/wifi/+", Wifi::DEFAULT_SETTINGS.connect(false)))
    //->structure(FileSystem::create("/io/fs/", args_parser->option("--mount", FOS_FS_MOUNT)))
#endif
#ifdef NATIVE
        ->structure(FileSystem::create("/io/fs/", args_parser->option("--mount", FOS_FS_MOUNT)))
    //->structure(Mqtt::create("//+/#"))
#endif
        ->model({ID("/model/sys")})
        ->structure(KeyValue::create("/console/#"))
        ->process(Console::create("/console/", "/terminal/:owner",
                                  Console::Settings(args_parser->option("--nest", "false") == "true",
                                                    args_parser->option("--ansi", "true") == "true",
                                                    args_parser->option("--strict", "false") == "true",
                                                    LOG_TYPES.to_enum(args_parser->option("--log", "INFO")))))
        ->eval([] { delete args_parser; })
        ->done("kernel_barrier");
  } catch (const std::exception &e) {
    LOG(ERROR, "[%s] !rCritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::silly_print("shutting down").c_str(),
        e.what());
  }
}

void loop() {
  // do nothing -- all looping handled by FhatOS scheduler
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#ifdef NATIVE
int main(const int argc, char **argv) {
  args_parser->init(argc, argv);
  if (args_parser->option("--help", "NO_HELP") != "NO_HELP") {
    const auto ansi = new Ansi();
    ansi->printf("%s: A Distributed Operating System\n", ansi->silly_print("FhatOS", true, true).c_str());
    ansi->printf("  --!b%-5s!!\n", "help");
    ansi->printf("  --!b%-5s!!=%5s\n", "ansi", "{!gtrue!!|!gfalse!!}");
    ansi->printf("  --!b%-5s!!=%5s\n", "log", "{!gINFO!!,!yWARN!!,!rERROR!!,!mDEBUG!!,!cTRACE!!,!bALL!!,!cNONE!!}");
    ansi->printf("  --!b%-5s!!=%5s\n", "mount", "{!glocal_dir_path!!}");
    delete ansi;
  } else {
    setup();
    loop();
  }
  return 0;
}
#endif