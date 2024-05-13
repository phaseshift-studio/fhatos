#ifndef fhatos_f_wifi_hpp
#define fhatos_f_wifi_hpp

#include <fhatos.hpp>
//
#include <process/process.hpp>
#include <structure/furi.hpp>
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

namespace fhatos {

class fWIFI : public KernelProcess {

private:
  explicit fWIFI(const ID &id = ID("wifi@127.0.0.1"),
                 const char *ssids = STR(WIFI_SSID),
                 const char *passwords = STR(WIFI_PASS))
      : KernelProcess(id) {
    this->ssids = ssids;
    this->passwords = passwords;
  }

protected:
  const char *ssids;
  const char *passwords;

public:
  static ID idFromIP(const String &user, const String &path = emptyString) {
    if (!fWIFI::singleton()->running())
      fWIFI::singleton()->setup();
    return {(user + "@" + fWIFI::ip().toString() +
             (path.isEmpty() ? emptyString : ("/" + path)))
                .c_str()};
  }

  static fWIFI *singleton(const ID &id = ID("wifi@127.0.0.1"),
                          const char *ssids = STR(WIFI_SSID),
                          const char *passwords = STR(WIFI_PASS)) {
    static fWIFI singleton = fWIFI(id, ssids, passwords);
    return &singleton;
  }

  static const IPAddress ip() { return WiFi.localIP(); }

  static const IPAddress resolve(const String &hostname) {
    IPAddress addr;
    WiFiClass::hostByName(hostname.c_str(), addr);
    return addr;
  }

  static const bool reconnect() { return WiFi.reconnect(); }

  void setup() override {
    KernelProcess::setup();
    this->setStation();
  }

  void stop() override {
    WiFi.disconnect();
    KernelProcess::stop();
  }

private:
  fWIFI *setAccessPoint(const char *ssid, const char *password,
                        const bool hideSSID = false,
                        const uint8_t maxConnections = 8) {
    WiFiClass::mode(WIFI_AP_STA);
    // WiFi.onSoftAPModeStationConnected(onNewStation);
    LOG(INFO, "[WIFI Access Point Configuration]\n");
    // WiFi.softAPConfig();
    if (!WiFi.softAP(ssid, password, hideSSID, maxConnections)) {
      LOG(ERROR, "Unable to create access point: %s\n", ssid);
    } else {
      LOG(NONE,
          "\tID:              %s\n"
          "\tSSID:            %s\n"
          "\tLocal IP:        %s\n"
          "\tMac Address:     %s\n"
          "\tBroadcast:       %s\n"
          "\tChannel:         %i\n"
          "\tMax connections: %i\n",
          this->id().toString().c_str(), ssid,
          WiFi.softAPIP().toString().c_str(), WiFi.softAPmacAddress().c_str(),
          hideSSID ? "false" : "true", WiFi.channel(), maxConnections);
    }

    WiFi.enableAP(true);
    return this;
  }

  fWIFI *setStation() {
    LOG(INFO, "!b[WIFI Station Configuration]!!\n");
    WiFiClass::mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    // WiFi.persistent(true);
    const char *delim = ":";
    char ssidsTemp[strlen(this->ssids) + 1];
    sprintf(ssidsTemp, "%s", this->ssids);
    char *ssid = strtok(ssidsTemp, delim);
    int i = 0;
    char *ssids_parsed[10];
    LOG(NONE, "\tWiFi SSIDs:\n");
    while (ssid != nullptr) {
      LOG(NONE, "\t\t%s\n", ssid);
      ssids_parsed[i++] = ssid;
      ssid = strtok(nullptr, delim);
    }
    i = 0;
    char passwordsTemp[50];
    sprintf(passwordsTemp, "%s", this->passwords);
    char *passwords_parsed[10];
    char *password = strtok(passwordsTemp, delim);
    while (password != nullptr) {
      passwords_parsed[i++] = password;
      password = strtok(nullptr, delim);
    }
    WIFI_MULTI_CLIENT multi;
    for (int j = 0; j < i; j++) {
      multi.addAP(ssids_parsed[j], passwords_parsed[j]);
    }
    uint8_t attempts = 0;
    while (attempts < 10) {
      attempts++;
      if (multi.run() == WL_CONNECTED) {
        this->_id =
            *(new ID(fWIFI::idFromIP("kernel", "wifi").toString().c_str()));
        WiFiClass::hostname(this->id().user().value());
        const bool mdnsStatus = MDNS.begin(this->id().user().value());
        LOG(NONE,
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
            this->id().toString().c_str(),
            WiFi.isConnected() ? "CONNECTED" : "DISCONNECTED",
            WiFi.SSID().c_str(), WiFi.macAddress().c_str(),
            WiFi.localIP().toString().c_str(), WiFi.getHostname(),
            mdnsStatus ? (this->id().user().value() + ".local").c_str()
                       : "<error>",
            WiFi.gatewayIP().toString().c_str(),
            WiFi.subnetMask().toString().c_str(),
            WiFi.dnsIP().toString().c_str(), WiFi.channel());
        if (!mdnsStatus) {
          LOG_TASK(ERROR, this, "Unable to create mDNS hostname %s\n",
                   this->id().user()->c_str());
        }
        LOG(INFO, "\tConnection attempts: %i\n", attempts);
        attempts = 100;
      }
    }
    if (attempts != 100) {
      LOG_TASK(ERROR, this, "Unable to connect to WIFI after %i attempts\n",
               attempts);
    }
    return this;
  }
};
} // namespace fhatos

#endif