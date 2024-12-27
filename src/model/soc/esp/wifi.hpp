
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

#include <fhatos.hpp>
#include <structure/stype/computed.hpp>
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
#error Architecture unrecognized by FhatOS
#endif

#define FOS_WIFI_CONNECT id_p(pattern()->resolve("./connect"))
#define FOS_WIFI_SSID id_p(pattern()->resolve("./ssid"))
#define FOS_WIFI_PASSWORD id_p(pattern()->resolve("./password"))
#define FOS_WIFI_MDNS id_p(pattern()->resolve("./mdns"))
#define FOS_WIFI_ID_ADDR id_p(pattern()->resolve("./ip_addr"))
#define FOS_WIFI_GATEWAY_ADDR id_p(pattern()->resolve("./gateway_addr"))
#define FOS_WIFI_SUBNET_MASK id_p(pattern()->resolve("./subnet_mask"))
#define FOS_WIFI_DNS_ADDR id_p(pattern()->resolve("./dns_addr"))

namespace fhatos {

  class Wifi : public Computed {

  public:
    struct Settings {
      bool connect_;
      string mdns_;
      string ssid_;
      string password_;
      Settings(const bool connect, const string &mdns, const string &ssid, const string &password) :
          connect_(connect), mdns_(mdns), ssid_(ssid), password_(password) %s;
    };

  protected:
    Settings settings_;
    explicit Wifi(const Pattern &pattern, const Settings &settings, const ID &id) :
        Computed(pattern, id), settings_(settings) {
      if (settings.connect_)
        this->connect_to_wifi_station();
      // TODO: flash/partition/0x44343
    }

  public:
    static ptr<Wifi> singleton(const Pattern &pattern, const Settings &settings, const ID &id = ID("")) {
      static ptr<Wifi> wifi = ptr<Wifi>(new Wifi(pattern, settings, id));
      return wifi;
    }

    // virtual List<ID_p> existing_ids(const fURI &match) override { return WIFI_IDS; }
    virtual void setup() override {
      Computed::setup();
      this->read_functions_->insert(
          {this->pattern(), [this](const fURI_p furi) {
             IdObjPairs map = IdObjPairs();
             ID_p current;
             if (FOS_WIFI_CONNECT->matches(*furi))
               map.push_back({FOS_WIFI_CONNECT, dool(WiFi.isConnected())});
             if (FOS_WIFI_SSID->matches(*furi))
               map.push_back({FOS_WIFI_SSID, str(this->settings_.ssid_)});
             if (FOS_WIFI_PASSWORD->matches(*furi))
               map.push_back({FOS_WIFI_PASSWORD, str(this->settings_.password_)});
             if (FOS_WIFI_MDNS->matches(*furi))
               map.push_back({FOS_WIFI_MDNS, vri(WiFi.getHostname())});
             if (FOS_WIFI_ID_ADDR->matches(*furi))
               map.push_back({FOS_WIFI_ID_ADDR, vri(WiFi.localIP().toString().c_str())});
             if (FOS_WIFI_GATEWAY_ADDR->matches(*furi))
               map.push_back({FOS_WIFI_GATEWAY_ADDR, vri(WiFi.gatewayIP().toString().c_str())});
             if (FOS_WIFI_SUBNET_MASK->matches(*furi))
               map.push_back({FOS_WIFI_SUBNET_MASK, vri(WiFi.subnetMask().toString().c_str())});
             if (FOS_WIFI_DNS_ADDR->matches(*furi))
               map.push_back({FOS_WIFI_DNS_ADDR, vri(WiFi.dnsIP().toString().c_str())});
             return map;
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !yread functions!! loaded\n", this->pattern()->toString().c_str());
      this->write_functions_->insert(
          {share(this->pattern()->resolve("./connect")), [this](const fURI_p furi, const Obj_p &obj) {
             if (obj->is_bool()) {
               if (obj->bool_value()) {
                 if (!WiFi.isConnected())
                   this->settings_.connect_ = connect_to_wifi_station();
               } else {
                 if (WiFi.isConnected())
                   WiFi.disconnect();
               }
             } else if (obj->is_noobj() && WiFi.isConnected())
               WiFi.disconnect();
             return List<Pair<ID_p, Obj_p>>{{id_p(this->pattern()->resolve("./connect")), dool(WiFi.isConnected())}};
           }});
      LOG_STRUCTURE(INFO, this, "!b%s !ywrite functions!! loaded\n",
                    this->pattern()->resolve("connect").toString().c_str());
    }

    void stop() override {
      Computed::stop();
      WiFi.disconnect();
    }

    //////////////////////////////////////////////////////////////////////////////////////////

  private:
    bool connect_to_wifi_station() const {
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      const char *delim = ":";
      char ssidsTemp[this->settings_.ssid_.length() + 1];
      sprintf(ssidsTemp, "%s", this->settings_.ssid_.c_str());
      char *ssid = strtok(ssidsTemp, delim);
      int i = 0;
      char *ssids_parsed[10];
      LOG_STRUCTURE(DEBUG, this, "\tWiFi SSIDs:\n");
      while (ssid != NULL) {
        LOG_STRUCTURE(DEBUG, this, "\t\t%s\n", ssid);
        ssids_parsed[i++] = ssid;
        ssid = strtok(NULL, delim);
      }
      i = 0;
      char passwordsTemp[50];
      sprintf(passwordsTemp, "%s", this->settings_.password_.c_str());
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
      WiFi.hostname(this->settings_.mdns_.c_str());
      uint8_t attempts = 0;
      while (attempts < 10) {
        attempts++;
        if (multi.run() == WL_CONNECTED) {
          const bool mdnsStatus = MDNS.begin(this->settings_.mdns_.c_str());
          LOG_STRUCTURE(INFO, this,
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
                        this->settings_.mdns_.c_str(), WiFi.isConnected() ? "CONNECTED" : "DISCONNECTED",
                        WiFi.SSID().c_str(), WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str(),
                        WiFi.getHostname(), mdnsStatus ? (this->settings_.mdns_ + ".local").c_str() : "<error>",
                        WiFi.gatewayIP().toString().c_str(), WiFi.subnetMask().toString().c_str(),
                        WiFi.dnsIP().toString().c_str(), WiFi.channel());
          if (!mdnsStatus) {
            LOG_STRUCTURE(WARN, this, "unable to create mDNS hostname %s\n", this->settings_.mdns_.c_str());
          }
          LOG_STRUCTURE(DEBUG, this, "connection attempts: %i\n", attempts);
          attempts = 100;
        }
      }
      if (attempts != 100) {
        LOG_STRUCTURE(ERROR, this, "unable to connect to WIFI after %i attempts\n", attempts);
        return false;
      }
      return true;
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
       this->vid()->toString().c_str(), ssid, WiFi.softAPIP().toString().c_str(),
       WiFi.softAPmacAddress().c_str(), hideSSID ? "false" : "true",
       WiFi.channel(), maxConnections);
 }

 WiFi.enableAP(true);
}*/
  };
} // namespace fhatos
#endif