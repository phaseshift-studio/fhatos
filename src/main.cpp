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
#include <language/types.hpp>
#include <structure/console/console.hpp>
#include <util/options.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <process/router/local_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
#ifdef NATIVE
#include <structure/kernel/f_kernel.hpp>


using namespace fhatos;


int main(int arg, char **argsv) {

  GLOBAL_OPTIONS->LOGGING = LOG_TYPE::INFO;
  GLOBAL_OPTIONS->PRINTING = Ansi<>::singleton();
  GLOBAL_OPTIONS->ROUTING = LocalRouter::singleton();
  Scheduler::singleton();
  Parser::singleton();
  Types::singleton()->loadExt("/ext/process");
  Types::singleton()->loadExt("/ext/collection");
  LOG(INFO, ANSI_ART);
  try {
    fKernel<>::bootloader({
        // fWIFI::singleton(),
        // fKernel<>::singleton(),
        // fFS<>::singleton(),
        // fOTA<>::singleton(),
    });
    // fScheduler<>::singleton()->spawn(new fLog());
    // fScheduler<>::singleton()->spawn(fSerial<>::singleton());
    // fScheduler<>::singleton()->spawn(new fPing<>());
    // fScheduler<>::singleton()->spawn(fTelnet<>::singleton());
    Scheduler::singleton()->spawn(new Console());
    Scheduler::singleton()->barrier("no_processes", [] { return Scheduler::singleton()->count() == 0; });
  } catch (const fError &e) {
    LOG(ERROR, "main() error: %s\n", e.what());
    // LOG_EXCEPTION(e);
  }
};
#else
#include FOS_MODULE(kernel / f_kernel.hpp)
#include FOS_MODULE(io / net / f_wifi.hpp)
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_MQTT(mqtt_router.hpp)
#include <language/fluent.hpp>
#include <language/instructions.hpp>
using namespace fhatos;

void setup() {
  fKernel<>::bootloader({
      fWIFI::singleton(),
  });
}


void loop() {}
#endif
