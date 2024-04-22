#ifndef fhatos_kernel__wifi_hpp
#define fhatos_kernel__wifi_hpp

#include <fhatos.hpp>
#include <kernel/process/esp32/thread.hpp>
//
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
// #include <ESPAsyncTCP.h>
#define WIFI_MULTI_CLIENT ESP8266WiFiMulti
#elif defined(ESP32)
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#define WIFI_MULTI_CLIENT WiFiMulti
#else
#error Architecture unrecognized by this code.
#endif

namespace fhatos::kernel {

class WIFI : public Thread {

private:
  WIFI(const ID& id = ID("wifi"),
       const char *ssids = STR(WIFI_SSID),
       const char *passwords = STR(WIFI_PASS))
      : Thread(id) {
    this->ssids = ssids;
    this->passwords = passwords;
  }

protected:
  const char *ssids;
  const char *passwords;

public:
  inline static WIFI *singleton(const ID& id = ID("wifi"),
                                const char *ssids = STR(WIFI_SSID),
                                const char *passwords = STR(WIFI_PASS)) {
    static WIFI singleton = WIFI(id, ssids, passwords);
    return &singleton;
  }

  bool running() override { return WiFi.isConnected(); }

  IPAddress ip() { return WiFi.localIP(); }

  bool reconnect() { return WiFi.reconnect(); }

  void start() override { this->setStation(); }

  void stop() override {
    WiFi.disconnect();
    Thread::stop();
  }

private:
  WIFI *setAccessPoint(const char *ssid, const char *password,
                       const bool hideSSID = false,
                       const uint8_t maxConnections = 8) {
    WiFi.mode(WIFI_AP_STA);
    // WiFi.onSoftAPModeStationConnected(onNewStation);
    LOG(INFO, "[WIFI Access Point Configuration]\n");
    // WiFi.softAPConfig();
    if (!WiFi.softAP(ssid, password, hideSSID, maxConnections)) {
      LOG(ERROR, "Unable to create access point: %s\n", ssid);
    } else {
      LOG(INFO,
          "\tID:              %s\n"
          "\tSSID:            %s\n"
          "\tLocal IP:        %s\n"
          "\tMac Address:     %s\n"
          "\tBroadcast:       %s\n"
          "\tChannel:         %i\n"
          "\tMax connections: %i\n",
          this->id().c_str(), ssid, WiFi.softAPIP().toString().c_str(),
          WiFi.softAPmacAddress().c_str(), hideSSID ? "false" : "true",
          WiFi.channel(), maxConnections);
    }

    WiFi.enableAP(true);
    return this;
  }

  WIFI *setStation() {
    LOG(INFO, "!b[WIFI Station Configuration]!!\n");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    const char *delim = ":";
    char ssidsTemp[strlen(this->ssids) + 1];
    sprintf(ssidsTemp, this->ssids);
    char *ssid = strtok(ssidsTemp, delim);
    int i = 0;
    char *ssids_parsed[10];
    LOG(INFO, "\tWiFi SSIDs:\n");
    while (ssid != NULL) {
      LOG(INFO, "\t\t%s\n", ssid);
      ssids_parsed[i++] = ssid;
      ssid = strtok(NULL, delim);
    }
    i = 0;
    char passwordsTemp[50];
    sprintf(passwordsTemp, this->passwords);
    char *passwords_parsed[10];
    char *password = strtok(passwordsTemp, delim);
    while (password != NULL) {
      passwords_parsed[i++] = password;
      password = strtok(NULL, delim);
    }
    WIFI_MULTI_CLIENT multi;
    for (int j = 0; j < i; j++) {
      multi.addAP(ssids_parsed[j], passwords_parsed[j]);
    }
    WiFi.hostname(Helper::machine());
    uint8_t attempts = 0;
    while (attempts < 10) {
      attempts++;
      if (multi.run() == WL_CONNECTED) {
        this->__id = Helper::makeId("wifi");
        const bool mdnsStatus = MDNS.begin(Helper::machine());
        LOG(INFO,
            "\tID             : %s\n"
            "\tStatus         : %s\n"
            "\tSSID           : %s\n"
            "\tMAC address    : %s\n"
            "\tIP address     : %s\n"
            "\tHostname       : %s\n"
            "\tmDNS name      : %s\n"
            "\tGateway address: %s\n"
            "\tSubnet mask    : %s\n"
            "\tDNS address    : %s\n"
            "\tChannel        : %i\n",
            this->id().c_str(),
            WiFi.isConnected() ? "CONNECTED" : "DISCONNECTED",
            WiFi.SSID().c_str(), WiFi.macAddress().c_str(),
            WiFi.localIP().toString().c_str(), WiFi.getHostname(),
            mdnsStatus ? (String(Helper::machine()) + ".local").c_str()
                       : "<error>",
            WiFi.gatewayIP().toString().c_str(),
            WiFi.subnetMask().toString().c_str(),
            WiFi.dnsIP().toString().c_str(), WiFi.channel());
        if (!mdnsStatus) {
          LOGTASK(ERROR, this, "Unable to create mDNS hostname %s\n",
                  Helper::machine());
        }
        LOG(INFO, "\tConnection attempts: %i\n", attempts);
        attempts = 100;
      }
    }
    if (attempts != 100) {
      LOGTASK(ERROR, this, "Unable to connect to WIFI after %i attempts\n",
              attempts);
    }
    return this;
  }
};
} // namespace fhatos::kernel


#endif