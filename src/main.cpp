#include <fhatos.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/log/log.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include <kernel/structure/machine/device/io/net/ping.hpp>
#include <kernel/structure/machine/device/io/net/telnet.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/machine/device/io/serial/serial.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

using namespace fhatos::kernel;

Log<> *logger;

void setup() {
  ::Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler::singleton()->addProcess(WIFI::singleton());
  Scheduler::singleton()->addProcess(MQTT<Thread, Message<String>>::singleton());
  Scheduler::singleton()->addProcess(logger = new Log<>());
  Scheduler::singleton()->addProcess(new fhatos::kernel::Serial<>());
  Scheduler::singleton()->addProcess(new fhatos::kernel::Ping<>());
  Scheduler::singleton()->addProcess(Telnet<>::singleton());
  Scheduler::singleton()->addProcess(Scheduler::singleton());
}

void loop() {
  static int counter = 0;
  /*LocalRouter<StringMessage>::singleton()->publish(StringMessage(
      ID("self@127.0.0.1"), logger->id().extend("INFO"),
      String("!Mlogging!! !Rme!!ssage: !g") + counter++ + "!!\n",
     RETAIN_MESSAGE));*/
  if (counter++ == -1) {
    LocalRouter<Message<String>>::singleton()->publish(
        Message<String>("self@127.0.0.1", WIFI::idFromIP("ping"),
                      String("www.google.com"), RETAIN_MESSAGE));
  }
}