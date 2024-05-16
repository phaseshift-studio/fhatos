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
#include FOS_MODULE(io/net/f_ota.hpp)
#include FOS_MODULE(io/net/f_ping.hpp)
#include FOS_MODULE(io/net/f_telnet.hpp)
#include FOS_MODULE(io/fs/f_fs.hpp)
#include <language/fluent.hpp>
using namespace fhatos;

void setup() {
  fKernel<>::bootloader();
  //
  Scheduler::singleton()->spawn(fSoC<Thread>::singleton());
  Scheduler::singleton()->spawn(new fLog<>());
  Scheduler::singleton()->spawn(fSerial<>::singleton());
  Scheduler::singleton()->spawn(fTelnet<>::singleton());
}

/*void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  fKernel<> *kernel = fKernel<>::singleton();
  Scheduler *s = Scheduler::singleton();
  s->spawn(fWIFI::singleton());
  //s->spawn(FOS_DEFAULT_ROUTER::singleton());
  s->spawn(fSoC<Thread, FOS_DEFAULT_ROUTER>::singleton());
  //s->spawn(fOTA<Fiber>::singleton());
  s->spawn(new fLog<Coroutine, FOS_DEFAULT_ROUTER>());
  //s->spawn(fSerial<Fiber, FOS_DEFAULT_ROUTER>::singleton());
  s->spawn(fTelnet<>::singleton());

  s->spawn(fFS<>::singleton());
  s->spawn(new fPing<>());

}*/

void loop() {

}
