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
#ifndef fhatos_mem_hpp
#define fhatos_mem_hpp

#include <fhatos.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/external.hpp>
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


namespace fhatos {

  class SoC : public Actor<Coroutine, External> {

  protected:
    struct Settings {
      // Settings() = delete;
    
      struct Wifi {
        const char *md5name;
        const char *ssids;
        const char *passwords;
      } wifi_;
    } settings_;

    const static inline Settings DEFAULT_SETTINGS =
        Settings{.wifi_ = Settings::Wifi{.md5name = "fhatty", .ssids = STR(WIFI_SSID), .passwords = STR(WIFI_PASS)}};

    explicit SoC(const ID &id = "/soc/", const Settings &settings = DEFAULT_SETTINGS) : Actor(id), settings_(settings) {
      this->connect_to_wifi_station();
      // TODO: *pin/a0
      // TODO: flash/partition/0x44343
    }


    enum MEM_TYPE { INST, HEAP, PSRAM };
    void write_memory_stats(MEM_TYPE mem_type) {
      switch (mem_type) {
        case INST:
          this->write(id_p(this->id_->extend("inst")),
                      parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                            ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                            ESP.getSketchSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                     ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace())))))),
                      RETAIN_MESSAGE);
          break;
        case HEAP:
          this->write(id_p(this->id_->extend("heap")),
                      parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getHeapSize(),
                            ESP.getFreeHeap(),
                            ESP.getHeapSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize()))))),
                      RETAIN_MESSAGE);
          break;
        case PSRAM:
          this->write(id_p(this->id_->extend("psram")),
                      parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getPsramSize(),
                            ESP.getFreePsram(),
                            ESP.getPsramSize() == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize()))))),
                      RETAIN_MESSAGE);
          break;
      }
    }

  public:
    static ptr<SoC> singleton(const ID id = "/soc/", const Settings &settings = DEFAULT_SETTINGS) {
      static ptr<SoC> soc = ptr<SoC>(new SoC(id, settings));
      return soc;
    }

    // void publish_retained(const Subscription_p &subscription) override {
    //  Actor::publish_retained(subscription);
    //}

    void setup() override {
      Actor::setup();
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), parse("is(gte(0.0)).is(lte(100.0))"));
      // Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/mem_stat"),parse("~[total=>int[_],free=>int[_],used=>"
      // FOS_TYPE_PREFIX "real/%%[_]]"));
      this->write_memory_stats(INST);
      this->write_memory_stats(HEAP);
      this->write_memory_stats(PSRAM);
    }

    Obj_p read(const fURI_p &furi) override {
      this->write_memory_stats(furi->matches(this->id_->extend("inst/#"))   ? INST
                               : furi->matches(this->id_->extend("heap/#")) ? HEAP
                                                                            : PSRAM);
      return Actor::read(furi);
    }

    void stop() override {
      Actor::stop();
      WiFi.disconnect();
    }

    void connect_to_wifi_station() const {
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      const char *delim = ":";
      char ssidsTemp[strlen(this->settings_.wifi_.ssids) + 1];
      sprintf(ssidsTemp, this->settings_.wifi_.ssids);
      char *ssid = strtok(ssidsTemp, delim);
      int i = 0;
      char *ssids_parsed[10];
      LOG_ACTOR(DEBUG, this, "\tWiFi SSIDs:\n");
      while (ssid != NULL) {
        LOG_ACTOR(DEBUG, this, "\t\t%s\n", ssid);
        ssids_parsed[i++] = ssid;
        ssid = strtok(NULL, delim);
      }
      i = 0;
      char passwordsTemp[50];
      sprintf(passwordsTemp, this->settings_.wifi_.passwords);
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
      WiFi.hostname(this->settings_.wifi_.md5name);
      uint8_t attempts = 0;
      while (attempts < 10) {
        attempts++;
        if (multi.run() == WL_CONNECTED) {
          // this->__id = Helper::makeId("wifi");
          const bool mdnsStatus = MDNS.begin(this->settings_.wifi_.md5name);
          LOG_ACTOR(INFO, this,
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
                    this->id()->toString().c_str(), WiFi.isConnected() ? "CONNECTED" : "DISCONNECTED",
                    WiFi.SSID().c_str(), WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str(),
                    WiFi.getHostname(), mdnsStatus ? (string(this->settings_.wifi_.md5name) + ".local").c_str() : "<error>",
                    WiFi.gatewayIP().toString().c_str(), WiFi.subnetMask().toString().c_str(),
                    WiFi.dnsIP().toString().c_str(), WiFi.channel());
          if (!mdnsStatus) {
            LOG_ACTOR(WARN, this, "Unable to create mDNS hostname %s\n", this->settings_.wifi_.md5name);
          }
          LOG_ACTOR(DEBUG, this, "Connection attempts: %i\n", attempts);
          attempts = 100;
        }
      }
      if (attempts != 100) {
        LOG_ACTOR(ERROR, this, "Unable to connect to WIFI after %i attempts\n", attempts);
      }
    }

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
  };
} // namespace fhatos
#endif