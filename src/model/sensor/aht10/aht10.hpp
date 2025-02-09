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
#ifndef NATIVEd

#include "../../../fhatos.hpp"
#include "../../../lang/type.hpp"
#include "../../../lang/obj.hpp"
#include "../../../lang/mmadt/type.hpp"
#include "../../../util/obj_helper.hpp"
#include "../../../util/global.hpp"
#include "ext/ahtxx.hpp"
#ifdef ARDUINO
#include "ext/ahtxx.hpp"
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif

namespace fhatos {
  static ID_p AHT10_FURI = id_p("/fos/sensor/aht10");

  class AHT10 final {
  protected:
    static void refresh(const Rec_p &aht10) {
      if(!aht10->vid_)
        return;
      if(!GLOBAL::singleton()->exists(aht10->vid_))
        GLOBAL::singleton()->store(aht10->vid_,
                                   make_shared<AHTxx>(aht10->rec_get("config/addr")->int_value()));
      const auto ahtxx = GLOBAL::singleton()->load<ptr<AHTxx>>(aht10->vid_);
      aht10->rec_set("celsius", real(ahtxx->readTemperature()));
      aht10->rec_set("humidity", real(ahtxx->readHumidity()));
    }

  public:
    static void *import() {
      Typer::singleton()->save_type(AHT10_FURI, Obj::to_rec(
                                    {{"celsius", Obj::to_type(mmadt::CELSIUS_FURI)},
                                     {"humidity", Obj::to_type(mmadt::PERCENT_FURI)},
                                     {"config", Obj::to_rec({
                                          {"addr", Obj::to_type(mmadt::UINT8_FURI)},
                                          {"i2c", Obj::to_type(URI_FURI)}})}}));
      InstBuilder::build(AHT10_FURI->add_component("setup"))
          ->domain_range(AHT10_FURI, {1, 1}, AHT10_FURI, {1, 1})
          ->inst_f([](const Obj_p &aht10, const InstArgs &) {
            AHT10::refresh(aht10);
            return aht10;
          })->save();
      ////
      InstBuilder::build(AHT10_FURI->add_component("refresh"))
          ->domain_range(AHT10_FURI, {1, 1}, AHT10_FURI, {1, 1})
          ->inst_f([](const Obj_p &aht10, const InstArgs &) {
            AHT10::refresh(aht10);
            return aht10;
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
#endif
