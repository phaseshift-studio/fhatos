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

#define WIFI_ATTEMPTS 50
#define WIFI_TID FOS_URI "/net/wifi"

namespace fhatos {

  const ID_p WIFI_FURI = id_p(FOS_URI "/net/wifi");
  // const ID_p MAC_FURI = id_p(FOS_URI "/net/mac");
  const ID_p IP_FURI = id_p(FOS_URI "/net/ip");

  class WIFIx : public Rec {

  public:
    explicit WIFIx(const Rec_p &wifi_obj) : Rec(*wifi_obj) {}

    static Obj_p connect(const Obj_p &wifi_obj) {
      if(!WiFi.isConnected())
        WIFIx::connect_to_wifi_station(wifi_obj);
      else
        LOG_WRITE(WARN, wifi_obj.get(), L("!ywifi !ralready!! connected"));
      return wifi_obj;
    }

    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          *WIFI_FURI,
          InstBuilder::build(Typer::singleton()->vid->add_component(*WIFI_FURI))
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec(
                    {/*{vri(MAC_FURI), Obj::to_type(URI_FURI)},*/
                     {vri(IP_FURI), Obj::to_type(URI_FURI)},
                     {vri(WIFI_FURI), Obj::to_rec({{vri("halt"), __().else_(dool(true))},
                                                   {vri("config"), Obj::to_rec({{"ssid", Obj::to_type(URI_FURI)},
                                                                                {"password", Obj::to_type(STR_FURI)},
                                                                                {"mdns", __().else_(vri("none"))},
                                                                                {"on_connect", __().else_(__())}})}})},
                     {vri(WIFI_FURI->add_component("connect")),
                      InstBuilder::build(WIFI_FURI->add_component("connect"))
                          ->domain_range(WIFI_FURI, {1, 1}, WIFI_FURI, {1, 1})
                          ->inst_f([](const Obj_p &wifi_obj, const InstArgs &args) {
                            WIFIx::connect(wifi_obj);
                            return wifi_obj;
                          })
                          ->create()}});
              })
              ->create());
    }

  private:
    static void connect_to_wifi_station(const Obj_p &wifi_obj) {
      const ID_p wifi_obj_id = wifi_obj->vid;
      const Obj_p config = wifi_obj->rec_get("config");
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(wifi_obj_id) {
        WiFi.onEvent(
            [wifi_obj_id](WiFiEvent_t event, WiFiEventInfo_t info) {
              const Obj_p wifi_obj = ROUTER_READ(wifi_obj_id->query(WIFI_TID));
              LOG_WRITE(ERROR, wifi_obj.get(), L("reconnecting to wifi: %s", info.wifi_sta_disconnected.reason));
              WiFi.disconnect();
              WiFi.reconnect();
            },
            WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        /* WiFi.onEvent(
             [wifi_obj_id](WiFiEvent_t event, WiFiEventInfo_t info) {
               const Obj_p wifi_obj = ROUTER_READ(wifi_obj_id->query(WIFI_TID));
               const Obj_p on_connect = wifi_obj->rec_get("config/on_connect");
               if(!on_connect->is_noobj()) {
                 FEED_WATCHDOG();
                 mmADT::delift(on_connect)->apply(wifi_obj);
               }
             },
             WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);*/
      }
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      LOG_WRITE(INFO, wifi_obj.get(), L("!gc!yo!mn!rn!ge!yc!bt!ci!rn!mg!!"));
      uint8_t counter = 0;
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
      WiFi.setHostname(config->rec_get("mdns")->uri_value().toString().c_str());
      WiFi.begin(config->rec_get("ssid")->uri_value().toString().c_str(),
                 config->rec_get("password")->str_value().c_str());
      while(WiFi.status() != WL_CONNECTED) {
        Thread::delay(500);
        FEED_WATCHDOG();
        Ansi<>::singleton()->print(".");
        if(++counter > WIFI_ATTEMPTS)
          throw fError("!runable!y to create !bwifi connection!y after %d attempts!!", WIFI_ATTEMPTS);
      }
      Ansi<>::singleton()->print("\n");
      if(const bool mdns_status = MDNS.begin(config->rec_get("mdns")->uri_value().toString().c_str()); !mdns_status) {
        LOG_OBJ(WARN, wifi_obj, "unable to create mDNS hostname %s\n", config->rec_get("mdns")->uri_value().toString().c_str());
      }
      FEED_WATCHDOG();
      wifi_obj->obj_set("halt", dool(false));
      // wifi_obj->rec_set("mac", vri(WiFi.macAddress().c_str(), MAC_FURI));
      wifi_obj->rec_set("ip", vri(WiFi.localIP().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("host", vri(WiFi.getHostname()));
      wifi_obj->rec_set("gateway", vri(WiFi.gatewayIP().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("subnet", vri(WiFi.subnetMask().toString().c_str(), IP_FURI));
      wifi_obj->rec_set("dns", vri(WiFi.dnsIP().toString().c_str(), IP_FURI));
      string wifi_str = PrintHelper::pretty_print_obj(wifi_obj, 3);
      StringHelper::prefix_each_line(FOS_TAB_1, &wifi_str);
      LOG_OBJ(INFO, wifi_obj, "\n%s\n", wifi_str.c_str());
      /////////////////////////////////
      Thread::yield();
      FEED_WATCHDOG();
      Thread::delay(1000);
      const Obj_p on_connect = config->rec_get("on_connect");
      if(!on_connect->is_noobj()) {
        FEED_WATCHDOG();
        mmADT::delift(on_connect)->apply(wifi_obj);
      }
    }
  };
} // namespace fhatos
#endif
#endif
