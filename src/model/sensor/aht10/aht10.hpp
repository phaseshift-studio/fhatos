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
#ifndef fhatos_aht10_hpp
#define fhatos_aht10_hpp
#ifndef NATIVE

#include "../../../fhatos.hpp"
#include "../../../lang/type.hpp"
#include "../../../lang/obj.hpp"
#include "../../../util/obj_helper.hpp"
#include "../../../structure/router.hpp"
#ifdef ESP_ARCH
#include "ext/ahtxx.hpp"
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif
//#ifdef NATIVE
//#include <wiringPi.h>
//#endif

namespace fhatos {
  class AHT10 final : public Rec {
  protected:
   ptr<AHTxx> ahtxx_;
  
  public:
    explicit AHT10(const ID &id, const ID& i2c_id, const int addr, const ptr<AHTxx>& ahtxx) : Rec(rmap({
          {"config",Obj::to_rec({{"addr",jnt(addr)},{"i2c",from(vri(i2c_id))}})},
        {"humidity",
          InstBuilder::build(id.extend("humidity"))
          ->domain_range(OBJ_FURI, {0, 1}, REAL_FURI, {1, 1})
          ->inst_f([this](const Obj_p &, const InstArgs &) {
            return real(this->ahtxx_->readHumidity());
          })
          ->create()},
        {"temp",
          InstBuilder::build(id.extend("temp"))
          ->domain_range(OBJ_FURI, {0, 1}, REAL_FURI, {1, 1})
         ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
             return real(this->ahtxx_->readTemperature());
          })
          ->create()}}), OType::REC, REC_FURI, id_p(id)),
          ahtxx_(ahtxx) {
    }

    static ptr<AHT10> create(const ID &id, const ID& i2c_id, const int addr) {
      ptr<AHTxx> athxx = make_shared<AHTxx>(addr, AHT1x_SENSOR);
      const Rec_p i2c = Router::singleton()->read(id_p(i2c_id));
      const uint8_t i2c_sda = i2c->rec_get("sda")->int_value();
      const uint8_t i2c_scl = i2c->rec_get("scl")->int_value();
      if(athxx->begin(i2c_sda,i2c_scl) != true) {
        throw fError::create(id.toString(),"!runable to connect!! to !yaht10 sensor!! on !bi2c!g[!ysda:!!%i,!yscl:!!%i!g]!!", i2c_sda, i2c_scl);
      }
      return std::make_shared<AHT10>(id,i2c_id,addr,athxx);
    }

    static void *import(const ID &lib_id = "/io/lib/aht10") {
      //Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      //BCODE_PROCESSOR(OBJ_PARSER("celcius -> |celcius?real<=real()[is(gte(−273.15))]"));
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_args(rec({{"id",Obj::to_type(URI_FURI)},{"i2c_id", Obj::to_type(URI_FURI)},{"addr",jnt(AHTXX_ADDRESS_X38)}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return AHT10::create(
              ID(args->arg("id")->uri_value()),
            args->arg("i2c_id")->uri_value(),
            args->arg("addr")->int_value());
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
#endif