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
#include FOS_FILE_SYSTEM(fs.hpp)

#ifdef NATIVE
#include <model/distributed_memory.hpp>
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
            ->with_log_level(LOG_TYPES.toEnum(args.option("--log", "INFO")))
            ->displaying_splash(ANSI_ART)
            ->displaying_notes("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !ynoobj!!")
            ->displaying_notes("Use !b:help!! for !yconsole commands!!")
                    ////////////////////////////////////////////////////////////
            ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
            ->using_router(Router::singleton("/sys/router/"))
                    ////////////////////////////////////////////////////////////
            ->boot<SharedMemory>(SharedMemory::create("/sys/memory/shared"))
            ->boot<Types>(Types::singleton("/type/"))
            ->boot<Terminal>(Terminal::singleton("/io/terminal/"))
            ->boot<Parser>(Parser::singleton("/sys/lang/parser/"))
#ifdef NATIVE
            ->boot<FileSystem>(FileSystem::singleton("/io/fs"))
            ->boot<DistributedMemory>(DistributedMemory::create("/sys/memory/cluster"))
#else
                    ->boot<FileSystem>(FileSystem::singleton("/io/fs", "/fs"))
#endif
            ->boot<Console>(Console::create("/home/root/repl/"))
            ->model({ID("/mod/proc")})
            ->initial_terminal_owner("/home/root/repl/")
            ->done("kernel_barrier");
  } catch (const std::exception &e) {
    LOG(ERROR, "[%s] !rCritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::sillyPrint("shutting down").c_str(), e.what());
  }
}

void loop() {
  // do nothing -- all looping handled by FhatOS scheduler
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#ifdef NATIVE

int main(int argc, char **argv) {
  args.init(argc, argv);
  setup();
  loop();
  return 0;
}

#endif
