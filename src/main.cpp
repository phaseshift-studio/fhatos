#include <fhatos.hpp>

#include FOS_MODULE(io/net/f_wifi.hpp)
#include <kernel/structure/f_soc.hpp>
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

#define MAIN_ROUTER LocalRouter<>

using namespace fhatos::kernel;

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler<MAIN_ROUTER> *s = Scheduler<MAIN_ROUTER>::singleton();
  s->spawn(fWIFI::singleton());
  s->spawn(MAIN_ROUTER::singleton());
  s->spawn(fSoC<Thread, MAIN_ROUTER>::singleton());
  s->spawn(fOTA<Fiber>::singleton());
  //s->spawn(new fLog<Coroutine, MAIN_ROUTER>());
  //s->spawn(fSerial<Fiber, MAIN_ROUTER>::singleton());
  s->spawn(new fPing<Fiber, MAIN_ROUTER>());
  s->spawn(fTelnet<Thread, MAIN_ROUTER>::singleton());
}

void loop() {
  // static int counter = 0;
  // if (counter < 10)
  //  fSerial<Fiber, String, MAIN_ROUTER>::println("testing...");
  /*LocalRouter<StringMessage>::singleton()->publish(StringMessage(
      ID("self@127.0.0.1"), logger->id().extend("INFO"),
      String("!Mlogging!! !Rme!!ssage: !g") + counter++ + "!!\n",
     RETAIN_MESSAGE));*/
  /*if (counter++ == -1) {
    LocalRouter<Message<String>>::singleton()->publish(
        Message<String>("self@127.0.0.1", fWIFI::idFromIP("ping"),
                        String("www.google.com"), RETAIN_MESSAGE));
  }*/
}