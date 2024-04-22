#include <fhatos.hpp>
#include <kernel/process/actor/router/router.hpp>
#include <kernel/structure/machine/device/io/net/mqtt/mqtt.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

using namespace fhatos::kernel;

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  LOG(NONE, ANSI_ART);
  Scheduler::singleton()->addProcess(WIFI::singleton());
  Scheduler::singleton()->addProcess(MQTT<Thread, StringMessage>::singleton());
  Scheduler::singleton()->setup();
}

void loop() {}