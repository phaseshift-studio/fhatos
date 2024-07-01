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

#ifndef fhatos_f_ota_hpp
#define fhatos_f_ota_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)
#include <ArduinoOTA.h>

namespace fhatos {

template <typename PROCESS = Fiber, typename ROUTER = Router> class fOTA : public PROCESS {

private:
  explicit fOTA(const ID &id = Router::mintID("kernel", "ota"),
       const uint16_t port = 3232)
      : PROCESS(id), port(port) {
    ArduinoOTA.setHostname(id.toString().c_str());
    ArduinoOTA.setPort(this->port);
    // ArduinoOTA.setMdnsEnabled(true);
  }
  uint16_t port;

public:
  inline static fOTA *singleton() {
    static fOTA singleton = fOTA();
    return &singleton;
  }

  fOTA *onStart(const Runnable startFunction) {
    ArduinoOTA.onStart(startFunction);
    return this;
  };
  fOTA *onProgress(const BiConsumer<uint32_t, uint32_t> progressFunction) {
    ArduinoOTA.onProgress(
        [progressFunction](uint32_t progress, uint32_t total) {
          progressFunction(progress, total);
        });
    return this;
  };
  fOTA *onError(const Consumer<ota_error_t> errorFunction) {
    ArduinoOTA.onError(
        [errorFunction](ota_error_t error) { errorFunction(error); });
    return this;
  }
  fOTA *onEnd(const Runnable endFunction) {
    ArduinoOTA.onEnd([endFunction]() { endFunction(); });
    return this;
  };
  virtual void stop() override {
    ArduinoOTA.end();
    PROCESS::stop();
  }
  virtual void setup() override {
    PROCESS::setup();
    ArduinoOTA.begin();
    LOG(INFO, "!b[OTA Configuration]!!\n");
    LOG(NONE,
        "\tID            : %s\n"
        "\tHost          : %s:%i\n"
        "\tUpdate command: %s\n",
        //"\tPartition     : %s\n",
        this->id().toString().c_str(),
        ArduinoOTA.getHostname().c_str(), this->port,
        ArduinoOTA.getCommand() == 0 ? "U_FLASH" : "U_FS"
        /*,ArduinoOTA.getPartitionLabel().isEmpty()
            ? "<none>"
            : ArduinoOTA.getPartitionLabel().c_str()*/);
  }
  virtual void loop() override {
    PROCESS::loop();
    ArduinoOTA.handle();
  };
};
} // namespace fhatos

#endif