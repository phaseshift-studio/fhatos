#ifndef fhatos_kernel__f_ota_hpp
#define fhatos_kernel__f_ota_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <ArduinoOTA.h>

namespace fhatos::kernel {

template <typename PROCESS = Fiber> class fOTA : public PROCESS {

private:
  fOTA(const ID &id = fWIFI::idFromIP("kernel", "ota"),
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

  fOTA *onStart(const Void0 startFunction) {
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
  fOTA *onEnd(const Void0 endFunction) {
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
} // namespace fhatos::kernel

#endif