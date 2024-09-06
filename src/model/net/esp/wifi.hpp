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
#ifndef fhatos_wifi_hpp
#define fhatos_wifi_hpp

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
#error Architecture unrecognized by this FhatOS deployment.
#endif

#include <fhatos.hpp>
#include <furi.hpp>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/key_value.hpp>
#include <model/terminal.hpp>

namespace fhatos {
class Wifi : public Actor<Coroutine,KeyValue> {

private:
  Wifi(const ID& id = ID("/sys/wifi"), const List<Pair<string,string>>& ssids_passwords = {}) :  Actor(id) {
    this->ssids_passwords = ssids_passwords;
  }

protected:
  List<Pair<string,string>> ssids_passwords;

public:
  inline static Wifi *singleton(const ID& id = ID("/sys/wifi"),
                               const List<Pair<string,string>>& ssids_passwords = {}) {
    static Wifi singleton = Wifi(id, ssids_passwords);
    return &singleton;
  }

 virtual bool active() override { return WiFi.isConnected(); }

  IPAddress ip() { return WiFi.localIP(); }

  bool reconnect() { return WiFi.reconnect(); }

  virtual void setup() override {
      Actor::setup();
      this->setStation();
  }

  virtual void stop() override {
    WiFi.disconnect();
    Actor::stop();
  }

private:
  void setAccessPoint(const char *ssid, const char *password,
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
          this->id().toString().c_str(), ssid, WiFi.softAPIP().toString().c_str(),
          WiFi.softAPmacAddress().c_str(), hideSSID ? "false" : "true",
          WiFi.channel(), maxConnections);
    }

    WiFi.enableAP(true);
  }

  void setStation() {
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
        //this->__id = Helper::makeId("wifi");
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
            this->_id->toString().c_str(),
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
      LOG_PROCESS(ERROR, this, "Unable to connect to WIFI after %i attempts\n",
              attempts);
    }
  }
};
} // namespace fatpig
#endif