#ifndef ota_hpp
#define ota_hpp

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
    Log::LOGGER(INFO,this, "\n\t!g[!bover-the-air setup!g]!!\n"
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