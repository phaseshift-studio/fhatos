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
#include "../../io/i2c/i2c.hpp"
#ifndef NATIVE

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../model/fos/sys/typer/typer.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../model.hpp"
#include "ext/ahtxx.hpp"
#ifdef ARDUINO
#include "ext/ahtxx.hpp"
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif

namespace fhatos {
  static ID_p AHT10_FURI = id_p("/fos/sensor/aht10");

  class AHT10 final : Model<AHT10> {
  protected:
    AHTxx ahtxx;

  public:
    explicit AHT10(const uint8_t addr) : ahtxx(AHTxx(addr)) {}

    static ptr<AHT10> create_state(const Obj_p &aht10) {
      I2C::get_state(Router::singleton()->read(aht10->rec_get("config/i2c")->uri_value()));
      return make_shared<AHT10>(aht10->rec_get("config/addr")->int_value());
    }

    static Obj_p refresh_inst(const Obj_p &aht10, const InstArgs &) {
      const ptr<AHT10> aht10_state = AHT10::get_state(aht10);
      aht10->rec_set("celsius", real(aht10_state->ahtxx.readTemperature(), CELSIUS_FURI));
      aht10->rec_set("humidity", real(aht10_state->ahtxx.readHumidity(), PERCENT_FURI));
      return aht10;
    }

    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          *AHT10_FURI, Obj::to_rec({{"celsius", Obj::to_type(CELSIUS_FURI)},
                                    {"humidity", Obj::to_type(PERCENT_FURI)},
                                    {"config", Obj::to_rec({{vri("addr"), Obj::to_type(UINT8_FURI)},
                                                            {vri("i2c"), Obj::to_type(URI_FURI)},
                                                            {vri(AHT10_FURI->add_component("refresh")),
                                                             InstBuilder::build(AHT10_FURI->add_component("refresh"))
                                                                 ->domain_range(AHT10_FURI, {1, 1}, AHT10_FURI, {1, 1})
                                                                 ->inst_f([](const Obj_p &aht10, const InstArgs &args) {
                                                                   return AHT10::refresh_inst(aht10, args);
                                                                 })
                                                                 ->create()}})}}));
    }
  };
} // namespace fhatos
#endif
#endif
