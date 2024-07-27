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
#include <process/x_process.hpp>
// routers
#include <process/router/local_router.hpp>
#include <process/router/meta_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
#include FOS_FILE_SYSTEM(filesystem.hpp)
// utilities
#include <language/types.hpp>
#include <structure/console/console.hpp>
#include <structure/io/terminal.hpp>

#ifndef FOS_ROUTERS
#ifdef NATIVE
#define FOS_ROUTERS                                                                                                    \
  LocalRouter::singleton("/sys/router/local"), MqttRouter::singleton("/sys/router/global"),                            \
      MetaRouter::singleton("/sys/router/meta")
#else
#define FOS_ROUTERS LocalRouter::singleton("/sys/router/local")
#endif
#endif

using namespace fhatos;
void setup() {
  try {
    Kernel::build()
        ->initialPrinter(Ansi<>::singleton())
        ->initialLogLevel(INFO)
        ->withSplash(ANSI_ART)
        ->withNote("Use !bØ!! for noobj abc")
        ->withNote("Use !y:help!! for console commands")
        ->onBoot(Scheduler::singleton("/sys/scheduler/"), //
                 {FOS_ROUTERS, //
                  Terminal::singleton("/sys/io/terminal/"), //
                  Types::singleton("/sys/lang/type/"), //
                  Parser::singleton("/sys/lang/parser/"), //
                  FileSystem::singleton("/sys/io/fs"), //
                  new Console("/home/root/repl/")})
        ->loadModules({"/ext/process"})
        ->defaultOutput("/home/root/repl/")
        ->done("kernel_barrier");
    Terminal::printer<>()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n", Ansi<>::sillyPrint("shutting down").c_str());
  } catch (const std::exception &e) {
    LOG(ERROR, "[%s] !rCritical!! !mFhat!gOS!! !rerror!!: %s\n", Ansi<>::sillyPrint("shutting down").c_str(), e.what());
    throw;
  }
}

void loop() {
  // do nothing -- all looping handled by FhatOS scheduler
}

#ifdef NATIVE
int main(int, char **) {
  setup();
  loop();
}
#endif
