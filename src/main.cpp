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
#include <process/router/local_router.hpp>
#include <structure/console/console.hpp>

#include FOS_PROCESS(scheduler.hpp)
#ifdef NATIVE
#include FOS_MODULE(kernel/f_kernel.hpp)
#include FOS_MODULE(kernel/f_lang.hpp)
#include FOS_MODULE(io/f_log.hpp)
//#include FOS_MODULE(f_bcode.hpp)
using namespace fhatos;

using namespace fhatos;
int main(int arg, char **argsv) {
  _logging = LOG_TYPE::NONE;
  try {
    fKernel<>::bootloader({
    //fWIFI::singleton(),
    fKernel<>::singleton(),
    LocalRouter<>::singleton(),
    fScheduler<>::singleton(),
    //fFS<>::singleton(),
    //fOTA<>::singleton(),
    fLang<>::singleton()
});
    fScheduler<>::singleton()->spawn(new fLog());
    //fScheduler<>::singleton()->spawn(fSerial<>::singleton());
    //fScheduler<>::singleton()->spawn(new fPing<>());
    //fScheduler<>::singleton()->spawn(fTelnet<>::singleton());
    Scheduler::singleton()->spawn(new Console<CPrinter>());
    Scheduler::singleton()->join();
  } catch (fError* e) {
    LOG(ERROR,"main() error: %s\n",e->what());
   // LOG_EXCEPTION(e);
  }
};
#else
#include FOS_MODULE(kernel/f_kernel.hpp)
#include FOS_MODULE(kernel/f_lang.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)
#include <structure/f_soc.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_ROUTER(mqtt_router.hpp)
#include FOS_MODULE(io/f_log.hpp)
#include <structure/f_soc.hpp>
#include FOS_MODULE(io/f_serial.hpp)
#include FOS_MODULE(io/net/f_ping.hpp)
#include FOS_MODULE(io/net/f_telnet.hpp)
#include FOS_MODULE(io/fs/f_fs.hpp)
#include <language/fluent.hpp>
#include <language/instructions.hpp>
using namespace fhatos;

void setup() {
  fKernel<>::bootloader({
      fWIFI::singleton(),
      fKernel<>::singleton(),
      FOS_DEFAULT_ROUTER::singleton(),
      fScheduler<>::singleton(),
      fMemory<>::singleton(),
      fFS<>::singleton(),
      fOTA<>::singleton(),
      fLang<>::singleton()
  });
  fScheduler<>::singleton()->spawn(fSoC<>::singleton());
  fScheduler<>::singleton()->spawn(new fLog());
  fScheduler<>::singleton()->spawn(fSerial<>::singleton());
  fScheduler<>::singleton()->spawn(new fPing<>());
  fScheduler<>::singleton()->spawn(fTelnet<>::singleton());

}


void loop() {

}
#endif
