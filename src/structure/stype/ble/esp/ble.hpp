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
#ifndef fhatos_ble_hpp
#define fhatos_ble_hpp

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <fhatos.hpp>
#include <language/processor/processor.hpp>
#include <structure/stype/external.hpp>

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED)
#error Bluetooth not enabled. Enable with -DCONFIG_BT_ENABLE=1.
#endif

// #define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

namespace fhatos {

#define BT_DISCOVER_TIME 10000

  class BLE : public External {
  protected:
    BLEServer *server_{};
    BLEService *service_{};
    explicit BLE(const Pattern &pattern = "/io/ble/#") : External(pattern) {}

    void setup() override {
      External::setup();
      BLEDevice::init(this->pattern_->toString().c_str());
      const BLEUUID service_uuid = BLEUUID::fromString(StringHelper::format("0x%s", this->pattern_->toString().c_str()));
      this->server_ = BLEDevice::createServer();

      this->service_ = this->server_->createService(service_uuid);
      this->service_->start();
      BLEAdvertising *advertising = BLEDevice::getAdvertising();
      advertising->addServiceUUID(service_uuid);
      advertising->setScanResponse(true);
      advertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
      advertising->setMinPreferred(0x12);
      BLEDevice::startAdvertising();
      //advertising->start();
      /////
      this->read_functions_->insert(
          {furi_p(this->pattern()->resolve("./+")), [this](const fURI_p &furi) {
             List<Pair<ID_p, Obj_p>> list;
             BLECharacteristic *c = this->service_->getCharacteristic(furi->toString());
             if (c) {
               LOG_STRUCTURE(DEBUG, this, "Reading BLE characteristic: %s\n", c->toString().c_str());
               const string v = c->getValue();
               const Obj_p obj = Obj::deserialize(
                   ptr<Pair<uint32_t, fbyte *>>((new Pair<uint32_t, fbyte *>(v.length(), (fbyte *) v.c_str()))));
               list.push_back(Pair<ID_p, Obj_p>(id_p(*furi), obj));
             }
             return list;
           }});

      this->write_functions_->insert(
          {furi_p(this->pattern()->resolve("./+")), [this](const fURI_p &furi, const Obj_p &obj) {
             BLECharacteristic *c = this->service_->createCharacteristic(
                 furi->toString(), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
             c->setValue(string((char *) obj->serialize()->second));
             //c->setBroadcastProperty(true);
             LOG_STRUCTURE(DEBUG, this, "Writing BLE characteristic: %s\n", c->toString().c_str());
             return List<Pair<ID_p, Obj_p>>();
           }});
    }

    void stop() override {
      External::stop();
      this->service_->stop();
      this->server_->disconnect(this->server_->getConnId());
    }

  public:
    static ptr<BLE> create(const Pattern &pattern = "/io/ble/#") {
      ptr<BLE> bt = ptr<BLE>(new BLE(pattern));
      return bt;
    }
  };
} // namespace fhatos
#endif