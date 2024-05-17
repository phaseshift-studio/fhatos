#include <fhatos.hpp>

#include FOS_MODULE(kernel/f_kernel.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)
#include <structure/f_soc.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_ROUTER(mqtt_router.hpp)
#include FOS_MODULE(io/f_log.hpp)
#include FOS_MODULE(io/f_serial.hpp)
#include FOS_MODULE(io/net/f_ping.hpp)
#include FOS_MODULE(io/net/f_telnet.hpp)
#include FOS_MODULE(io/fs/f_fs.hpp)
#include <language/fluent.hpp>
using namespace fhatos;
// #include <fhatos_native.hpp>


//#endif

void setup() {
  fKernel<>::bootloader({
      fWIFI::singleton(),
      fKernel<>::singleton(),
      FOS_DEFAULT_ROUTER::singleton(),
      fScheduler<>::singleton(),
      fMemory<>::singleton(),
      fFS<>::singleton(),
      fOTA<>::singleton()
  });
  fScheduler<>::singleton()->spawn(fSoC<>::singleton());
  fScheduler<>::singleton()->spawn(new fLog());
  fScheduler<>::singleton()->spawn(fSerial<>::singleton());
  fScheduler<>::singleton()->spawn(new fPing<>());
  fScheduler<>::singleton()->spawn(fTelnet<>::singleton());
}


void loop() {

}
