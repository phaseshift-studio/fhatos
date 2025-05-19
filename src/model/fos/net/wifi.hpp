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
#ifndef NATIVE
#include "../../../fhatos.hpp"
#include "../../../util/print_helper.hpp"
#include "../../fos/sys/scheduler/scheduler.hpp"
#include "../../model.hpp"
#include "../sys/typer/typer.hpp"
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

/*#define FOS_WIFI_CONNECT id_p(pattern->resolve("./connect"))
#define FOS_WIFI_SSID id_p(pattern->resolve("./ssid"))
#define FOS_WIFI_PASSWORD id_p(pattern->resolve("./password"))
#define FOS_WIFI_MDNS id_p(pattern->resolve("./mdns"))
#define FOS_WIFI_ID_ADDR id_p(pattern->resolve("./ip_addr"))
#define FOS_WIFI_GATEWAY_ADDR id_p(pattern->resolve("./gateway_addr"))
#define FOS_WIFI_SUBNET_MASK id_p(pattern->resolve("./subnet_mask"))
#define FOS_WIFI_DNS_ADDR id_p(pattern->resolve("./dns_addr"))*/

namespace fhatos {

  const ID_p WIFI_FURI = id_p(FOS_URI "/net/wifi");
  const ID_p MAC_FURI = id_p(FOS_URI "/net/mac");
  const ID_p IP_FURI = id_p(FOS_URI "/net/ip");

  class WIFIx : public Model<WIFIx> {

  public:
    static Obj_p connect_inst(const Obj_p &wifi_obj, const InstArgs &) {
      if(!WiFi.isConnected())
        WIFIx::connect_to_wifi_station(wifi_obj);
      else
        LOG_WRITE(WARN, wifi_obj.get(), L("!ywifi !ralready!! connected"));
      return wifi_obj;
    }

    static Rec_p obj(const std::initializer_list<Pair<const string, Obj_p>> &map, const ID &value_id) {
      return Obj::to_rec(map, WIFI_FURI, id_p(value_id));
    }

    static Rec_p obj(const Rec_p &config, const ID &value_id) {
      return Obj::to_rec(config->rec_value(), WIFI_FURI, id_p(value_id));
    }

    static void load_module() {
      const ID module_id = Typer::singleton()->vid->extend("module").extend(*WIFI_FURI);
      Typer::singleton()->obj_set(
          ID("module").extend(*WIFI_FURI),
          InstBuilder::build(module_id)
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec(
                    {{vri(*WIFI_FURI), Obj::to_rec({{"halt", __().else_(dool(true))},
                                                    {"config", Obj::to_rec({{"ssid", Obj::to_type(URI_FURI)},
                                                                            {"password", Obj::to_type(STR_FURI)},
                                                                            {"mdns", __().else_(vri("none"))}})}})},
                     {vri(*MAC_FURI), Obj::to_type(URI_FURI)},
                     {vri(*IP_FURI), Obj::to_type(URI_FURI)},
                     {vri(WIFI_FURI->add_component("connect")),
                      InstBuilder::build(WIFI_FURI->add_component("connect"))
                          ->domain_range(WIFI_FURI, {1, 1}, WIFI_FURI, {1, 1})
                          ->inst_f([](const Obj_p &wifi_obj, const InstArgs &args) {
                            Scheduler::singleton()->for_scheduler.push_back([wifi_obj, args] {
                              WIFIx::connect_inst(wifi_obj, args);
                              wifi_obj->obj_set("halt", dool(false));
                              return wifi_obj;
                            });
                            LOG_WRITE(INFO, wifi_obj.get(), L("!gc!yo!mn!rn!ge!yc!bt!ci!rn!mg!!"));
                            /*while(wifi_obj->obj_get("halt")->bool_value()) {
                              Ansi<>::singleton()->print(".");
                              Thread::yield();
                              Thread::delay(2000);
                            }*/
                            return wifi_obj;
                          })
                          ->create()}});
              })
              ->create());
    }


    static void *import(const Obj_p &wifi_config = nullptr) {
      //////////////////////
      Typer::singleton()->save_type(*WIFI_FURI,
                                    Obj::to_rec({{"halt", __().else_(dool(true))},
                                                 {"config", Obj::to_rec({{"ssid", Obj::to_type(URI_FURI)},
                                                                         {"password", Obj::to_type(STR_FURI)},
                                                                         {"mdns", __().else_(vri("none"))}})}}));
      Typer::singleton()->save_type(*MAC_FURI, Obj::to_type(URI_FURI));
      Typer::singleton()->save_type(*IP_FURI, Obj::to_type(URI_FURI));
      ///////////////////////////////////////////////////////
      InstBuilder::build(WIFI_FURI->add_component("connect"))
          ->domain_range(WIFI_FURI, {1, 1}, WIFI_FURI, {1, 1})
          ->inst_f([](const Obj_p &text, const InstArgs &args) { return WIFIx::connect_inst(text, args); })
          ->save();
      ///////////////////////////////////////////////////////
      if(wifi_config && !wifi_config->is_noobj()) {
        const Obj_p &wifi_obj = WIFIx::obj({{"halt", dool(false)}, {"config", wifi_config->clone()}}, "/io/wifi");
        LOG_OBJ(INFO, wifi_obj, "!ywifi connection!! attempt: !b%s!!\n",
                wifi_config->rec_get("ssid")->uri_value().toString().c_str());
        WIFIx::connect_inst(wifi_obj, Obj::to_inst_args());
      }
      return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////

  private:
    static bool connect_to_wifi_station(const Obj_p &wifi_obj) {
      const Obj_p config = wifi_obj->rec_get("config");
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      const char *delim = ":";
      char *ssidsTemp = strdup(config->rec_get("ssid")->uri_value().toString().c_str());
      char *ssid = strtok(ssidsTemp, delim);
      int i = 0;
      char *ssids_parsed[10];
      // LOG_STRUCTURE(DEBUG, this, "\tWiFi SSIDs:\n");
      while(ssid != nullptr) {
        // LOG_STRUCTURE(DEBUG, this, "\t\t%s\n", ssid);
        ssids_parsed[i++] = ssid;
        ssid = strtok(nullptr, delim);
      }
      i = 0;
      char passwordsTemp[50];
      sprintf(passwordsTemp, "%s", config->rec_get("password")->str_value().c_str());
      char *passwords_parsed[10];
      char *password = strtok(passwordsTemp, delim);
      while(password != nullptr) {
        passwords_parsed[i++] = password;
        password = strtok(nullptr, delim);
      }
      WIFI_MULTI_CLIENT multi;
      for(int j = 0; j < i; j++) {
        multi.addAP(ssids_parsed[j], passwords_parsed[j]);
      }
      WiFi.hostname(config->rec_get("mdns")->uri_value().toString().c_str());
      uint8_t attempts = 0;
      while(attempts < 10) {
        attempts++;
        if(multi.run() == WL_CONNECTED) {
          const string mdns_name = config->rec_get("mdns")->uri_value().toString();
          if(const bool mdns_status = MDNS.begin(mdns_name.c_str()); !mdns_status) {
            LOG_OBJ(WARN, wifi_obj, "unable to create mDNS hostname %s\n", mdns_name.c_str());
          }
          LOG_OBJ(DEBUG, wifi_obj, "connection attempts: %i\n", attempts);
          attempts = 100;
        }
      }
      if(attempts != 100) {
        LOG_OBJ(ERROR, wifi_obj, "unable to connect to WIFI after %i attempts\n", attempts);
        return false;
      }
      wifi_obj->rec_set("mac", vri(WiFi.macAddress().c_str(), MAC_FURI));
      wifi_obj->rec_set("ip", vri(WiFi.localIP().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("host", vri(WiFi.getHostname()));
      wifi_obj->rec_set("gateway", vri(WiFi.gatewayIP().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("subnet", vri(WiFi.subnetMask().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("dns", vri(WiFi.dnsIP().toString().c_str(), IP_FURI));
      string wifi_str = PrintHelper::pretty_print_obj(wifi_obj, 3);
      StringHelper::prefix_each_line(FOS_TAB_1, &wifi_str);
      WiFi.begin();
      LOG_OBJ(INFO, wifi_obj, "\n%s\n", wifi_str.c_str());
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
       this->vid->toString().c_str(), ssid, WiFi.softAPIP().toString().c_str(),
       WiFi.softAPmacAddress().c_str(), hideSSID ? "false" : "true",
       WiFi.channel(), maxConnections);
 }

 WiFi.enableAP(true);
}*/
  };
} // namespace fhatos
#endif
#endif
