#include <fhatos.hpp>

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

#define MAIN_ROUTER LocalRouter<>

using namespace fhatos;

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler<> *s = Scheduler<>::singleton();
  s->spawn(fWIFI::singleton());
  //s->spawn(MAIN_ROUTER::singleton());
  //s->spawn(fSoC<Thread, MAIN_ROUTER>::singleton());
  //s->spawn(fOTA<Fiber>::singleton());
  s->spawn(new fLog<Coroutine, MAIN_ROUTER>());
  //s->spawn(fSerial<Fiber, MAIN_ROUTER>::singleton());
  s->spawn(fTelnet<>::singleton());
  //s->spawn(new fPing());
  s->spawn(fFS<>::singleton());

}

void loop() {

}
