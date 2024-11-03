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
#ifndef fhatos_base_driver_hpp
#define fhatos_base_driver_hpp

#include <language/obj.hpp>
#include <language/type.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  enum class PROTOCOL { PWM, GPIO, I2C, SPI, MQTT };

  static const auto ProtocolTypes =
      Enums<PROTOCOL>({{PROTOCOL::GPIO, "gpio"}, {PROTOCOL::PWM, "pwm"}, {PROTOCOL::I2C, "i2c"}, {PROTOCOL::SPI, "spi"},
                       {PROTOCOL::MQTT, "mqtt"}});

  //static const ID_p DRIVER_TYPE = id_p(FOS_TYPE_PREFIX "/rec/driver/");
  static const ID_p PROTOCOL_TYPE_PREFIX = id_p(FOS_TYPE_PREFIX "/uri/protocol/");
  static const auto DRIVER_FURI = make_shared<ID>(FOS_TYPE_PREFIX "/rec/driver/");

  class fDriver : public ObjWrap {
  protected:
    ID_p type_;
    Rec_p internal_;

  public:
    explicit fDriver(const ID_p &request_id, const ID_p &response_id, const List_p<Inst_p> &instructions,
                     const ID_p &type) :
      ObjWrap(type), type_(type) {
      this->internal_ = Obj::to_rec({{vri("request"), vri(request_id)},
                                     {vri("response"), vri(response_id)},
                                     {vri("insts"), Obj::to_lst(instructions)}});
    }

  public:
    void setup() const {
      const List_p<Obj_p> inst_list = this->internal_->rec_get(vri("insts"))->lst_value();
      Type::singleton()->start_progress_bar(inst_list->size() + 1);
      for (const auto &inst: *inst_list) {
        Type::singleton()->save_type(id_p(*inst->type()), inst);
      }
      Type::singleton()->save_type(this->type_, this->internal_);
      Type::singleton()->end_progress_bar(
          StringHelper::format("!b%s !yinsts!! loaded\n", this->type()->toString().c_str()));

    }

    fURI_p type() const override {
      return this->type_;
    }

    BCode_p inst(const ID_p &inst_ref) const {
      for (const auto &[fid, fbcode]: *this->internal_->rec_value()) {
        if (fid->uri_value().equals(*inst_ref))
          return fbcode;
      }
      return noobj();
    }

    Rec_p to_rec() const override {
      return this->internal_->as(this->type_);
    }
  };

  using fDriver_p = shared_ptr<fDriver>;
}
#endif