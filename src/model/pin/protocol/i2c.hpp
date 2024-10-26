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
#ifndef fhatos_i2c_hpp
#define fhatos_i2c_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <structure/stype/computed.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#include <structure/router.hpp>
#endif

namespace fhatos {
  void request() {}
  void recv(int len) {}
  class I2C {

    class Master {
    public:
      virtual void init(const uint8_t sda, const uint8_t scl) = 0;
      virtual void begin_write(const uint8_t address) = 0;
      virtual void end_write(const bool release_lock) = 0;
      virtual void request_read(const uint8_t address, const int length, const bool release_lock) = 0;
      virtual int available_to_read() = 0;
      virtual fbyte read() = 0;
      virtual void write(const fbyte *data, const int length) = 0;
    };

    class Slave {
    public:
      virtual void init(const uint8_t sda, const uint8_t scl, const uint8_t address) = 0;
      virtual fbyte read() = 0;
      virtual void write(const fbyte *data, const int length) = 0;
      virtual void on_recv(const BCode_p &code) = 0;
    };
  };

#ifdef ESP_ARCH
  class ArduinoI2C : public I2C {
    class Master {
    public:
      virtual void init(const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) { Wire.begin(sda, scl, freq); }
      virtual void begin_write(const uint8_t address) { Wire.beginTransmission(address); }
      virtual void end_write(const bool release_lock) { Wire.endTransmission(release_lock); }
      virtual void request_read(const uint8_t address, const int length, const bool release_lock) {
        Wire.requestFrom(address, length, release_lock);
      }
      virtual int available_to_read() { return Wire.available(); }
      virtual fbyte read() { return Wire.read(); }
      virtual void write(const fbyte *data, const int length) { Wire.write(data, length); }
    };

    class Slave {

    public:
      virtual void init(const uint8_t address, const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) {
        Wire.begin(address, sda, scl, freq);
      }
      virtual fbyte read() { return Wire.read(); }
      virtual void write(const fbyte *data, const int length) { Wire.write(data, length); }
      virtual void on_recv(const BCode_p &code) { Wire.onReceive(recv); }
      virtual void on_request(const BCode_p &code) { Wire.onRequest(request); };
    };
  };
#endif

  class fURII2C : public I2C {


    class Master {
      fURI_p prefix_;
      ID_p master_;
      ID_p current_slave_;
      ID_p current_slave_write_;
      ID_p current_slave_read_;

      Master(const fURI &prefix) :
          prefix_(furi_p(prefix)), master_(nullptr), current_slave_(nullptr), current_slave_write_(nullptr),
          current_slave_read_(nullptr) {}


    public:
      virtual void init(const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) { 
        this->master_ = id_p(this->prefix_->resolve(string("./master/") + to_string(sda)));
       }
      virtual void begin_write(const uint8_t address) {
        this->current_slave_ = id_p(this->prefix_->resolve(string("./") + to_string(address)));
        this->current_slave_write_ = id_p(this->current_slave_->extend("write"));
        this->current_slave_read_ = id_p(this->current_slave_->extend("read"));
        router()->write(this->current_slave_, jnt(0));
      }
      virtual void end_write(const bool release_lock) {
        if (release_lock) {
          router()->write(this->current_slave_, release_lock ? jnt(-1) : jnt(0));
          if (release_lock) {
            this->current_slave_ = nullptr;
            this->current_slave_write_ = nullptr;
            this->current_slave_read_ = nullptr;
          }
        }
      }
      virtual void request_read(const uint8_t address, const int length, const bool release_lock) {
        router()->write(this->current_slave_read_, jnt(length), TRANSIENT_MESSAGE);
      }
      virtual int available_to_read() { return router()->read(this->current_slave_)->int_value(); }
      virtual fbyte read() { (fbyte) router()->read(this->current_slave_read_)->int_value(); }
      virtual void write(const fbyte *data, const int length) {
        router()->write(this->current_slave_write_, str(string((char *) data, length)), TRANSIENT_MESSAGE);
      }
    };

    class Slave {

      fURI_p prefix_;
      ID_p master_;
      ID_p slave_;
      ID_p slave_write_;
      ID_p slave_read_;

    public:
      virtual void init(const uint8_t address, const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) {
          this->slave_ = id_p(this->prefix_->resolve(string("./slave/") + to_string(sda)));
      }
      virtual fbyte read() { (fbyte) router()->read(this->slave_read_)->int_value(); }
      virtual void write(const fbyte *data, const int length) {
        router()->write(this->slave_write_, str(string((char *) data, length)));
      }
      virtual void on_recv(const BCode_p &code) {
        // Wire.onReceive(recv);
      }
      virtual void on_request(const BCode_p &code){
          // Wire.onRequest(request);
      };
    };
  };
} // namespace fhatos
#endif
