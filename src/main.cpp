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
// scheduler
#include FOS_PROCESS(scheduler.hpp)
// routers
#include <process/router/local_router.hpp>
#include <process/router/meta_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
// utilities
#include <language/types.hpp>
#include <structure/console/console.hpp>
#include <structure/io/terminal.hpp>
#include <util/options.hpp>

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
  GLOBAL_OPTIONS->LOGGING = LOG_TYPE::INFO;
  GLOBAL_OPTIONS->PRINTING = Ansi<>::singleton();
  /***
   Kernel::begin();
   Kernel::boot_loader()
   Kernel::load_procs();
   Kernel::load_modules();
   Kernel::end();
   ***/
  try {
    Terminal::printer<>()->print(ANSI_ART);
    Terminal::printer<>()->printf(FOS_TAB_4 "Use %s for noobj\n", Obj::to_noobj()->toString().c_str());
    Scheduler::singleton("/sys/scheduler/")
        ->onBoot({FOS_ROUTERS, //
                  Terminal::singleton("/sys/io/terminal/"), //
                  Types::singleton("/sys/lang/type/"), //
                  Parser::singleton("/sys/lang/parser/"), //
                  new Console("/home/root/repl/")});
    //////
    Types::singleton()->loadExt("/ext/process");
    Terminal::currentOut(share(ID("/home/root/repl/")));
    Scheduler::singleton()->barrier("main_barrier");
  } catch (const std::exception &e) {
    // LOG_EXCEPTION(e);
    LOG(ERROR, "[!rS!gH!yU!mT!yT!bI!rN!gG !cD!mO!gW!bN!!] Critical FhatOS error: %s\n", e.what());
    throw;
  }
}

void loop() { throw fError("Global loop() should never be called (regardless of underlying platform)"); }

#ifdef NATIVE
int main(int arg, char **argsv) {
  setup();
  loop();
}
#endif
