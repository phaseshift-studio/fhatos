#include <fhatos.hpp>
#include <kernel/process/router/local_router.hpp>
#include <kernel/process/router/mqtt_router.hpp>
#include <kernel/structure/machine/device/io/f_log.hpp>
#include <kernel/structure/machine/device/io/f_serial.hpp>
#include <kernel/structure/machine/device/io/net/f_ping.hpp>
#include <kernel/structure/machine/device/io/net/f_telnet.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

#define MAIN_ROUTER MqttRouter<>

using namespace fhatos::kernel;

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler<MAIN_ROUTER> *s = Scheduler<MAIN_ROUTER>::singleton();
  s->spawn(fWIFI::singleton());
  s->spawn(MAIN_ROUTER::singleton());
  // s->spawn(fMQTT<Thread, String>::singleton());
  s->spawn(new fLog<Coroutine, String, MAIN_ROUTER>());
  s->spawn(fSerial<Fiber, String, MAIN_ROUTER>::singleton());
  s->spawn(new fPing<Fiber, String, MAIN_ROUTER>());
  s->spawn(fTelnet<Thread, String, MAIN_ROUTER>::singleton());
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