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
//#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#define WIFI_MULTI_CLIENT WiFiMulti
#else
#error Architecture unrecognized by this FhatOS deployment.
#endif

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/key_value.hpp>

#define WIFI_NAME STR(fhatty)

namespace fhatos {
class Wifi : public Actor<Coroutine,KeyValue> {

protected:
  List<Pair<string,string>> ssids_passwords;
  const char *ssids;
  const char *passwords;

private:
  Wifi(const ID& id = ID("/sys/wifi"), const char *ssids = STR(WIFI_SSID),  const char *passwords = STR(WIFI_PASS))
      : Actor(id) {
    this->ssids = ssids;
    this->passwords = passwords;
      }

public:
  inline static ptr<Wifi> singleton(const ID& id = ID("/sys/wifi"),
                               const char *ssids = STR(WIFI_SSID),
                                    const char *passwords = STR(WIFI_PASS)) {
    static ptr<Wifi> singleton = ptr<Wifi>(new Wifi(id, ssids,passwords));
    return singleton;
  }

 //virtual bool active() override { return WiFi.isConnected(); }

 // IPAddress ip() { return WiFi.localIP(); }

 // bool reconnect() { return WiFi.reconnect(); }

  virtual void setup() override {
      Actor::setup();
      this->setStation();
  }

  virtual void stop() override {
    WiFi.disconnect();
    Actor::stop();
  }

private:
  /*void setAccessPoint(const char *ssid, const char *password,
                       const bool hideSSID = false,
                       const uint8_t maxConnections = 8) {
    WiFi.mode(WIFI_AP_STA);
    // WiFi.onSoftAPModeStationConnected(onNewStation);
    // WiFi.softAPConfig();
    if (!WiFi.softAP(ssid, password, hideSSID, maxConnections)) {
      LOG_ACTOR(ERROR, this, "Unable to create access point: %s\n", ssid);
    } else {
      LOG_ACTOR(INFO, this, 
          "\n[WIFI Access Point Configuration]\n"
          "\t!yID:              !m%s\n"
          "\t!ySSID:            !m%s\n"
          "\t!yLocal IP:        !m%s\n"
          "\t!yMac Address:     !m%s\n"
          "\t!yBroadcast:       !m%s\n"
          "\t!yChannel:         !m%i\n"
          "\t!yMax connections: !m%i!!\n",
          this->id()->toString().c_str(), ssid, WiFi.softAPIP().toString().c_str(),
          WiFi.softAPmacAddress().c_str(), hideSSID ? "false" : "true",
          WiFi.channel(), maxConnections);
    }

    WiFi.enableAP(true);
  }*/

  void setStation() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    const char *delim = ":";
    char ssidsTemp[strlen(this->ssids) + 1];
    sprintf(ssidsTemp, this->ssids);
    char *ssid = strtok(ssidsTemp, delim);
    int i = 0;
    char *ssids_parsed[10];
    LOG_ACTOR(DEBUG,this, "\tWiFi SSIDs:\n");
    while (ssid != NULL) {
      LOG_ACTOR(DEBUG, this, "\t\t%s\n", ssid);
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
    WiFi.hostname(WIFI_NAME);
    uint8_t attempts = 0;
    while (attempts < 10) {
      attempts++;
      if (multi.run() == WL_CONNECTED) {
        //this->__id = Helper::makeId("wifi");
        const bool mdnsStatus =false;// MDNS.begin(WIFI_NAME);
        LOG_ACTOR(INFO,this,
            "\n\t!g[!bWIFI Station Configuration!g]!!\n"
            "\t!yID             : !m%s\n"
            "\t!yStatus         : !m%s\n"
            "\t!ySSID           : !m%s\n"
            "\t!yMAC address    : !m%s\n"
            "\t!yIP address     : !m%s\n"
            "\t!yHostname       : !m%s\n"
            "\t!ymDNS name      : !m%s\n"
            "\t!yGateway address: !m%s\n"
            "\t!ySubnet mask    : !m%s\n"
            "\t!yDNS address    : !m%s\n"
            "\t!yChannel        : !m%i!!\n",
            this->id()->toString().c_str(),
            WiFi.isConnected() ? "CONNECTED" : "DISCONNECTED",
            WiFi.SSID().c_str(), WiFi.macAddress().c_str(),
            WiFi.localIP().toString().c_str(), WiFi.getHostname(),
            mdnsStatus ? (String(WIFI_NAME) + ".local").c_str()
                       : "<error>",
            WiFi.gatewayIP().toString().c_str(),
            WiFi.subnetMask().toString().c_str(),
            WiFi.dnsIP().toString().c_str(), WiFi.channel());
        if (!mdnsStatus) {
          LOG_ACTOR(WARN, this, "Unable to create mDNS hostname %s\n",
                  WIFI_NAME);
        }
        LOG_ACTOR(DEBUG, this, "Connection attempts: %i\n", attempts);
        attempts = 100;
      }
    }
    if (attempts != 100) {
      LOG_ACTOR(ERROR, this, "Unable to connect to WIFI after %i attempts\n",
              attempts);
    }
  }
};
} // namespace fatpig
#endif