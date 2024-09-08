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
#include <model/shared_memory.hpp>
#include <model/distributed_memory.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)

#ifndef NATIVE
#include <model/net/esp/wifi.hpp>
#include <model/memory/esp32/memory.hpp>
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT "tmp"
#else
#define FOS_FS_MOUNT "/"
#endif

namespace fhatos {
  class ArgvParser {
    Map<const string, string> _map = Map<const string, string>();

  public:
    void init(const int &argc, char **argv) {
      for (int i = 1; i < argc; ++i) {
        const string temp = string(argv[i]);
        size_t j = temp.find_first_of('=');
        if (j != string::npos) {
          string key = temp.substr(0, j);
          string value = temp.substr(j + 1);
          this->_map.insert({key, value});
        } else {
          string key = temp;
          string value = "";
          this->_map.insert({key, value});
        }
      }
    }

    string option(const string &option, const char *orElse) const {
      return this->_map.count(option) ? this->_map.at(option) : orElse;
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
    Kernel::build()
        ->using_printer(Ansi<>::singleton())
        ->with_ansi_color(args.option("--ansi", "true") == "true")
        ->with_log_level(LOG_TYPES.toEnum(args.option("--log", "INFO")))
        ->displaying_splash(ANSI_ART)
        ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!")
        ->displaying_notes("Use !b:help!! for !yconsole commands!!")
        ////////////////////////////////////////////////////////////
        ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
        ->using_router(Router::singleton("/sys/router/#"))
        ////////////////////////////////////////////////////////////
        #ifndef NATIVE
        ->boot<Wifi>(Wifi::singleton("/net/wifi"))
        #endif
        ->boot<SharedMemory>(SharedMemory::create("/memory/shared/", "+"))
        ->boot<Types>(Types::singleton("/type/"))
        ->boot<Terminal>(Terminal::singleton("/terminal/"))
        ->boot<Parser>(Parser::singleton("/parser/"))
        #ifndef NATIVE
         ->boot<Memory>(Memory::singleton("/memory/soc/"))
        #endif
        ->boot<FileSystem>(FileSystem::singleton("/io/fs/", args.option("--mount",FOS_FS_MOUNT)))
        ->boot<DistributedMemory>(DistributedMemory::create("/memory/cluster/"))
        ->boot<Console>(Console::create("/home/root/repl/"))
       // ->model({ID("/model/sys"), ID("/model/pubsub")})
        ->initial_terminal_owner("/home/root/repl/")
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
    printf("FhatOS: A Distributed Operating System\n");
    printf("  --%-5s=%5s\n", "ansi", "{true|false}");
    printf("  --%-5s=%5s\n", "log", "{INFO,ERROR,DEBUG,TRACE,ALL,NONE}");
    printf("  --%-5s=%5s\n", "mount", "{local_dir_path}");
  } else {
    setup();
    loop();
  }
  return 0;
}

#endif
