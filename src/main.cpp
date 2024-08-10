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
#include <structure/kernel.hpp>
// scheduler
#include FOS_PROCESS(scheduler.hpp)
#include <process/process.hpp>
#include <structure/stype/key_value.hpp>
// routers
#include <structure/rooter.hpp>
#include <structure/router/local_router.hpp>
#ifdef NATIVE
#include FOS_MQTT(mqtt_router.hpp)
#include FOS_FILE_SYSTEM(filesystem.hpp)
#include <structure/router/meta_router.hpp>
#endif
// utilities
#include <language/types.hpp>
#include <structure/console/console.hpp>
#include <structure/io/net/cluster.hpp>
#include <structure/io/terminal.hpp>

#ifndef FOS_ROUTERS
#ifdef NATIVE
#define FOS_ROUTERS                                                                                                    \
  LocalRouter::singleton("/sys/router/local"),                                                                         \
      MqttRouter::singleton("/sys/router/global", args.option("--mqtt", "localhost:1883").c_str()),                    \
      MetaRouter::singleton("/sys/router/meta")
#else
#define FOS_ROUTERS LocalRouter::singleton("/sys/router/local")
#endif
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
    Kernel::build()
        ->with_printer(Ansi<>::singleton())
        ->with_log_level(LOG_TYPES.toEnum(args.option("--log", "INFO").c_str()))
        ->initialRouter(LocalRouter::singleton())
        ->displaying_splash(ANSI_ART)
        ->displaying_notes("Use !b/type/noobj/[]!! for !ynoobj!!")
        ->displaying_notes("Use !b:help!! for !yconsole commands!!")
        ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
        ->using_router(Rooter::singleton("/sys/router/"))
        ->boot<Cluster, Fiber, Mqtt>(new Cluster("/io/cluster"))
        ->boot<Terminal, Thread, KeyValue>(Terminal::singleton("/io/terminal/"))
        ->boot<Types, Fiber, KeyValue>(Types::singleton("/type/"))
        ->boot<Parser, Coroutine, Empty>(Parser::singleton("/sys/lang/parser/"))
        ->boot<FileSystem, Fiber, Mount>(FileSystem::singleton("/io/fs"))
        ->boot<Console, Thread, Empty>(new Console("/home/root/repl/"))
        ->load_modules({ID("/mod/proc")})
        ->defaultOutput("/home/root/repl/")
        ->done("kernel_barrier");
    Options::singleton()->printer<>()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n",
                                              Ansi<>::sillyPrint("shutting down").c_str());
  } catch (const std::exception &e) {
    LOG(ERROR, "[%s] !rCritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::sillyPrint("shutting down").c_str(), e.what());
  }
  exit(1);
}
/*
 * // ->with_int_ctype(int)
// ->with_real_ctype(float)
// ->initialRouter(LocalRouter::singleton())
/*->onBoot(Scheduler::singleton("/sys/scheduler/"), //
          {FOS_ROUTERS, //
           Terminal::singleton("/sys/io/terminal/"), //
           Types::singleton("/sys/lang/type/"), //
           , //
#ifdef NATIVE
//                FileSystem::singleton(
//                   "/sys/io/fs", ID(fs::current_path()).resolve(args.option("--fs",
fs::current_path().c_str()))),
//                   //
#endif
           })*/

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
}
#endif
