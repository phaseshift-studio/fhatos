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

#pragma once
#ifndef fhatos_ota_hpp
#define fhatos_ota_hpp

#ifndef NATIVE

#include "../../../fhatos.hpp"
#include <ArduinoOTA.h>
#include "../../log.hpp"
#include  STR(../../../process/ptype/HARDWARE/thread.hpp)

#define DEFAULT_OTA_PORT 3232

namespace fhatos {
  class OTA final : public Thread {
  protected:

private:

explicit OTA(const ID &value_id, const Rec_p &config) : /* */
      Thread(Obj::to_rec({{"config",config}},REC_FURI,id_p(value_id))) {
        ArduinoOTA.setHostname(this->rec_get("config/host")->or_else(vri("fhatos_ota_host"))->uri_value().toString().c_str());
        ArduinoOTA.setPort(this->rec_get("config/port")->or_else(jnt(DEFAULT_OTA_PORT))->int_value());
}

public:
    static ptr<OTA> singleton(const ID &id, const Rec_p &config) {
      const auto console = ptr<OTA>(new OTA(id, config));
      return console;
    }

  OTA *onStart(const Runnable startFunction) {
    ArduinoOTA.onStart(startFunction);
    return this;
  };
  OTA *onProgress(const BiConsumer<uint32_t, uint32_t> progressFunction) {
    ArduinoOTA.onProgress(
        [progressFunction](uint32_t progress, uint32_t total) {
          progressFunction(progress, total);
        });
    return this;
  };
  OTA *onError(const Consumer<ota_error_t> errorFunction) {
    ArduinoOTA.onError(
        [errorFunction](ota_error_t error) { errorFunction(error); });
    return this;
  }
  OTA *onEnd(const Runnable endFunction) {
    ArduinoOTA.onEnd([endFunction]() { endFunction(); });
    return this;
  };
  void stop() override {
    ArduinoOTA.end();
    Thread::stop();
  }
  void setup() override {
    Thread::setup();
    ArduinoOTA.begin();
    Log::LOGGER(INFO,this, "\n\t!g[!bover-the-air config!g]!!\n"
        "\t!yid            : !m%s\n"
        "\t!yhost          : !m%s:%i\n"
        "\t!yupdate command: !m%s!!\n",
        //"\tPartition     : %s\n",
        this->vid_or_tid()->toString().c_str(),
        ArduinoOTA.getHostname().c_str(), this->rec_get("config/port")->or_else(jnt(DEFAULT_OTA_PORT))->int_value(),
        ArduinoOTA.getCommand() == 0 ? "U_FLASH" : "U_FS"
        /*,ArduinoOTA.getPartitionLabel().isEmpty()
            ? "<none>"
            : ArduinoOTA.getPartitionLabel().c_str()*/);
  }
  void loop() override { ArduinoOTA.handle(); };
};
} // namespace fatpig

#endif
#endif