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
#ifndef fhatos_mlx90614_hpp
#define fhatos_mlx90614_hpp
#include "../../io/i2c/i2c.hpp"
#ifndef NATIVE
#include "../../../../fhatos.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../model.hpp"
#ifdef ARDUINO
#include "ext/mlx90614xx.hpp"
#endif
#ifdef RASPBERRYPI
#include <wiringPi.h>
#endif

namespace fhatos {
  static ID_p MLX90614_FURI = id_p("/fos/sensor/mlx90614");

  class MLX90614 final : Model<MLX90614> {
  protected:
    MLX90614xx_I2C mlx90614xx;

  public:
    explicit MLX90614(const uint8_t addr) :
      mlx90614xx(MLX90614xx_I2C(addr)) {
        this->mlx90614xx.begin();
    }

    static ptr<MLX90614> create_state(const Obj_p &mlx90614) {
      I2C::get_or_create(Router::singleton()->read(id_p(mlx90614->rec_get("config/i2c")->uri_value())));
      return make_shared<MLX90614>(mlx90614->rec_get("config/addr")->int_value());
    }

    static Obj_p refresh_inst(const Obj_p &mlx90614, const InstArgs &) {
      const ptr<MLX90614> mlx90614_state = MLX90614::get_or_create(mlx90614);
      mlx90614->rec_set("ambient", real(mlx90614_state->mlx90614xx.getAmbientTempCelsius(), CELSIUS_FURI));
      mlx90614->rec_set("object", real(mlx90614_state->mlx90614xx.getObjectTempCelsius(), CELSIUS_FURI));
      return mlx90614;
    }

    static void *import() {
      Typer::singleton()->save_type(MLX90614_FURI, Obj::to_rec(
                                    {{"ambient", Obj::to_type(CELSIUS_FURI)},
                                     {"object", Obj::to_type(CELSIUS_FURI)},
                                     {"config", Obj::to_rec({
                                          {"addr", Obj::to_type(UINT8_FURI)},
                                          {"i2c", Obj::to_type(URI_FURI)}})}}));
      InstBuilder::build(MLX90614_FURI->add_component("refresh"))
          ->domain_range(MLX90614_FURI, {1, 1}, MLX90614_FURI, {1, 1})
          ->inst_f([](const Obj_p &mlx90614, const InstArgs &args) {
            return MLX90614::refresh_inst(mlx90614, args);
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos
#endif
#endif
