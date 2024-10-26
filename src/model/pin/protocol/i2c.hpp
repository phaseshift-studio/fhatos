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
#include <structure/router.hpp>
#ifdef ESP_ARCH
#include <Wire.h>
#endif

namespace fhatos {
  void request() {
  }

  void recv(int len) {
  }

  class I2C {
  public:
    class Master {
    public:
      virtual ~Master() = default;

      virtual void init(const uint8_t sda, const uint8_t scl, const uint32_t freq) = 0;

      virtual void begin_write(const uint8_t address) = 0;

      virtual void end_write(const bool release_lock) = 0;

      virtual void request_read(const uint8_t address, const int length, const bool release_lock) = 0;

      virtual int available_to_read() = 0;

      virtual fbyte read() = 0;

      virtual void write(const fbyte *data, const int length) = 0;
    };

    class Slave {
    public:
      virtual ~Slave() = default;

      virtual void init(const uint8_t sda, const uint8_t scl, const uint8_t address, const uint32_t freq) = 0;

      virtual fbyte read() = 0;

      virtual void write(const fbyte *data, const int length) = 0;

      virtual void on_recv(const BCode_p &code) = 0;

      virtual void on_request(const BCode_p &code) =0;
    };
  };

#ifdef ESP_ARCH
  class ArduinoI2C : public I2C {
    class Master : I2C::Master {
    public:
       void init(const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) override { Wire.begin(sda, scl, freq); }
       void begin_write(const uint8_t address) override{ Wire.beginTransmission(address); }
       void end_write(const bool release_lock) override{ Wire.endTransmission(release_lock); }
       void request_read(const uint8_t address, const int length, const bool release_lock)override {
        Wire.requestFrom(address, length, release_lock);
      }
       int available_to_read() override { return Wire.available(); }
       fbyte read() override { return Wire.read(); }
       void write(const fbyte *data, const int length) override { Wire.write(data, length); }
    };

    class Slave : I2C::Slave {
    public:
       void init(const uint8_t address, const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) override{
        Wire.begin(address, sda, scl, freq);
      }
       fbyte read() override { return (fbyte) Wire.read(); }
       void write(const fbyte *data, const int length) override { Wire.write(data, length); }
       void on_recv(const BCode_p &code) override { /*Wire.onReceive(recv);*/ }
       void on_request(const BCode_p &code) override { /*Wire.onRequest(request);*/ };
    };
  };
#endif

  class fURII2C : public I2C {
    class Master : public I2C::Master {
      fURI_p prefix_;
      ID_p master_;
      ID_p current_slave_;
      ID_p current_slave_write_;
      ID_p current_slave_read_;

      explicit Master(const fURI &prefix) : prefix_(furi_p(prefix)), master_(nullptr), current_slave_(nullptr),
                                            current_slave_write_(nullptr),
                                            current_slave_read_(nullptr) {
      }

    public:
      void init(const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) override {
        this->master_ = id_p(this->prefix_->resolve(string("./master/") + to_string(sda)));
      }

      void begin_write(const uint8_t address) override {
        this->current_slave_ = id_p(this->prefix_->resolve(string("./") + to_string(address)));
        this->current_slave_write_ = id_p(this->current_slave_->extend("write"));
        this->current_slave_read_ = id_p(this->current_slave_->extend("read"));
        router()->write(this->current_slave_, jnt(0));
      }

      void end_write(const bool release_lock) override {
        if (release_lock) {
          router()->write(this->current_slave_, release_lock ? jnt(-1) : jnt(0));
          if (release_lock) {
            this->current_slave_ = nullptr;
            this->current_slave_write_ = nullptr;
            this->current_slave_read_ = nullptr;
          }
        }
      }

      void request_read(const uint8_t address, const int length, const bool release_lock) override {
        router()->write(this->current_slave_read_, jnt(length), TRANSIENT_MESSAGE);
      }

      int available_to_read() override { return router()->read(this->current_slave_)->int_value(); }
      fbyte read() override { return (fbyte) router()->read(this->current_slave_read_)->int_value(); }

      void write(const fbyte *data, const int length) override {
        router()->write(this->current_slave_write_, str(string((char *) data, length)), TRANSIENT_MESSAGE);
      }
    };

    class Slave : public I2C::Slave {
      fURI_p prefix_;
      ID_p master_;
      ID_p slave_;
      ID_p slave_write_;
      ID_p slave_read_;

    public:
      void init(const uint8_t address, const uint8_t sda, const uint8_t scl, const uint32_t freq = 0) override {
        this->slave_ = id_p(this->prefix_->resolve(string("./slave/") + to_string(sda)));
      }

      fbyte read() override { return (fbyte) router()->read(this->slave_read_)->int_value(); }

      void write(const fbyte *data, const int length) override {
        router()->write(this->slave_write_, str(string((char *) data, length)));
      }

      void on_recv(const BCode_p &code) override {
        // Wire.onReceive(recv);
      }

      void on_request(const BCode_p &code) override {
        // Wire.onRequest(request);
      };
    };
  };
} // namespace fhatos
#endif
