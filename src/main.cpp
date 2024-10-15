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
#include <boot_loader.hpp>
#include <util/argv_parser.hpp>
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
//#include <model/ui/fhatui.hpp>
#ifdef ESP_ARCH
#include <EEPROM.h>
#include <model/soc/esp/gpio.hpp>
#include <model/soc/esp/interrupt.hpp>
#include <model/soc/esp/pwm.hpp>
#include <model/soc/esp/wifi.hpp>
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT string(getenv("FHATOS_HOME")).append("/data").c_str()
#else
#define FOS_FS_MOUNT "/"
#endif

using namespace fhatos;
ArgvParser *args_parser = new ArgvParser();
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void setup() {
  BootLoader::primary_boot(args_parser)
      //->process(StructureTree::create(ID("/ui/tree"), fURI("/sys/router/structure/")))
      ->done("kernel_barrier");
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