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

    virtual void setup() override {
      Actor::setup();
      Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/%"), parse("is(gte(0.0)).is(lte(100.0))"));
      //////////////
      /// MEMORY ///
      //////////////
      this->read_functions_.insert(
          {share(this->id()->resolve("memory/inst")), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {id_p(this->id()->resolve("memory/inst")),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]",
                        ESP.getSketchSize() + ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace(),
                        ESP.getSketchSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeSketchSpace()) /
                                                 ((float) (ESP.getSketchSize() + ESP.getFreeSketchSpace()))))))}};
           }});
      this->read_functions_.insert(
          {share(this->id()->resolve("memory/heap")), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {id_p(this->id()->resolve("memory/heap")),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getHeapSize(),
                        ESP.getFreeHeap(),
                        ESP.getHeapSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreeHeap()) / ((float) ESP.getHeapSize())))))}};
           }});
      this->read_functions_.insert(
          {share(this->id()->resolve("memory/psram")), [this](const fURI_p furi) {
             return Map<ID_p, Obj_p>{
                 {id_p(this->id()->resolve("memory/psram")),
                  parse("[total=>%i,free=>%i,used=>" FOS_TYPE_PREFIX "real/%%[%.2f]]", ESP.getPsramSize(),
                        ESP.getFreePsram(),
                        ESP.getPsramSize() == 0
                            ? 0.0f
                            : (100.0f * (1.0f - (((float) ESP.getFreePsram()) / ((float) ESP.getPsramSize())))))}};
           }});


      this->read_functions_.insert(
          {share(this->id()->resolve("pin/+")), [this](const fURI_p furi) {
             Map<ID_p, Obj_p> map;
             if (StringHelper::is_integer(furi->name())) {
               uint8_t pin_number = stoi(furi->name());
               map.insert({id_p(*furi), jnt(digitalRead(pin_number))});
             } else {
               for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                 map.insert({id_p(this->id()->resolve(fURI(string("pin/") + to_string(i)))), jnt(digitalRead(i))});
               }
             }
             return map;
           }});
      this->write_functions_.insert(
          {share(this->id()->resolve("pin/+")), [this](const fURI_p furi, const Obj_p &obj) {
             Map<ID_p, Obj_p> map;
             if (StringHelper::is_integer(furi->name())) {
               uint8_t pin_number = stoi(furi->name());
               digitalWrite(pin_number, obj->int_value());
               map.insert({id_p(*furi), obj});
             } else {
               for (uint8_t i = 0; i < NUM_DIGITAL_PINS; i++) {
                 map.insert({id_p(this->id()->resolve(fURI(string("pin/") + to_string(i)))), jnt(digitalRead(i))});
               }
             }
             return map;
           }});
      ///////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////


      ///////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////
      // Types::singleton()->save_type(id_p(FOS_TYPE_PREFIX
      // "rec/mem_stat"),parse("~[total=>int[_],free=>int[_],used=>" FOS_TYPE_PREFIX "real/%%[_]]"));
      //// this->write_memory_stats(INST);
      // this->write_memory_stats(HEAP);
      // this->write_memory_stats(PSRAM);
    }





  };
} // namespace fhatos
#endif