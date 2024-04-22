#include <fhatos.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/log/log.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

using namespace fhatos::kernel;

Log *logger;

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler::singleton()->addProcess(WIFI::singleton());
  Scheduler::singleton()->addProcess(MQTT<Thread, StringMessage>::singleton());
  Scheduler::singleton()->addProcess(logger = new Log());
  Scheduler::singleton()->setup();
}

void loop() {
  static int counter = 0;
  MetaRouter<StringMessage>::singleton()->publish(
      StringMessage(ID("self@127.0.0.1"), ID(logger->id().extend("INFO")),
                    String("logging message: ") + counter++ + "\n", false));
                    delay(150);
}