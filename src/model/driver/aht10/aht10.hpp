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
   AHTxx aht10 = AHTxx(AHTXX_ADDRESS_X38, AHT1x_SENSOR);
  
  public:
    explicit AHT10(const ID &value_id) : Rec(rmap({
          {"config",Obj::to_rec({{"i2c_addr",jnt(AHT10_ADDRESS_X39)}})},
        {"humidity",
          InstBuilder::build(value_id.extend("humidity"))
          ->domain_range(OBJ_FURI, {0, 1}, REAL_FURI, {1, 1})
          ->inst_f([this](const Obj_p &, const InstArgs &) {
            return real(this->aht10.readHumidity());
          })
          ->create()},
        {"read",
          InstBuilder::build(value_id.extend("temperature"))
          ->domain_range(OBJ_FURI, {0, 1}, REAL_FURI, {1, 1})
         ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
             return real(this->aht10.readTemperature());
          })
          ->create()}}), OType::REC, REC_FURI, id_p(value_id)) {
    }

    static ptr<AHT10> create(const ID &id) {
      const auto aht10_p = std::make_shared<AHT10>(id);
      while (aht10_p->aht10.begin() != true) {
        LOG_OBJ(ERROR,aht10_p,"could not connect to aht10 sensor");
        delay(5000);
      }
      return aht10_p;
    }

    static void *import(const ID &lib_id = "/io/lib/aht10") {
      //Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_args(rec({{"id",Obj::to_type(URI_FURI)},{"addr", Obj::to_type(INT_FURI)}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return AHT10::create(ID(args->arg("id")->uri_value()));
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
