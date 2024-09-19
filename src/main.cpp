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
#include <model/distributed_memory.hpp>
#include <model/shared_memory.hpp>
#include <model/sys.hpp>
#include <model/terminal.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)
#include <model/fs/base_fs.hpp>
#include <process/obj_process.hpp>

#ifndef NATIVE
#include <model/soc/esp32/soc.hpp>
#include <model/soc/pinout.hpp>
#include <model/soc/wifi.hpp>
#include "esp32/spiram.h"
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
        const string temp = string(argv[i]);
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

    string option(const string &option, const char *orElse) const {
      return this->map_.count(option) ? this->map_.at(option) : orElse;
    }
  };
} // namespace fhatos
using namespace fhatos;
static ArgvParser args = ArgvParser();

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void setup() {
  try {
    load_processor();
    load_threader();
    Kernel::build()
        ->using_printer(Ansi<>::singleton())
        ->with_ansi_color(args.option("--ansi", "true") == "true")
        ->with_log_level(LOG_TYPES.to_enum(args.option("--log", "INFO")))
        ->displaying_splash(ANSI_ART)
        ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!")
        ->displaying_notes("Use !b:help!! for !yconsole commands!!")
        ////////////////////////////////////////////////////////////
        ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
        ->using_router(Router::singleton("/sys/router/#"))
#ifdef NATIVE
        ->boot<System>(System::singleton())
#endif
        ////////////////////////////////////////////////////////////
        ->boot<SharedMemory>(SharedMemory::create("/var/", "+/#"))
        ->boot<Types>(Types::singleton("/type/"))
        ->boot<Terminal>(Terminal::singleton("/terminal/"))
        ->boot<Parser>(Parser::singleton("/parser/"))
#ifndef NATIVE
        ->structure(Pinout::singleton("/soc/pinout/#"))
        ->structure(Wifi::singleton("/soc/wifi/+"))
#endif
#ifdef NATIVE
        ->boot<FileSystem>(FileSystem::create("/io/fs/", args.option("--mount", FOS_FS_MOUNT)))
        ->boot<DistributedMemory>(DistributedMemory::create("/cluster/", "//+/#"))
        ->model({ID("/model/sys"), ID("/model/pubsub")})
#endif
        ->boot<Console>(Console::create("/home/root/repl/", "/terminal/"))
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
  args.init(argc, argv);
  if (args.option("--help", "NO_HELP") != "NO_HELP") {
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