#include <fhatos.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/log/log.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

using namespace fhatos::kernel;

Log<> *logger;
ID* loggerINFO;
ID self("self@127.0.0.1");

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler::singleton()->addProcess(WIFI::singleton());
  Scheduler::singleton()->addProcess(MQTT<Thread, StringMessage>::singleton());
  Scheduler::singleton()->addProcess(logger = new Log<>());
  loggerINFO = new ID(logger->id().extend("INFO"));
  Scheduler::singleton()->setup();
}

void loop() {
  static int counter = 0;
  String x = String("logging message: ") + counter++ + "\n";

  LocalRouter<StringMessage>::singleton()->publish(
      StringMessage(self, *loggerINFO,
                   x , false));
}