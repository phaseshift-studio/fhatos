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
#ifndef fhatos_fos_type_hpp
#define fhatos_fos_type_hpp

#include "../../fhatos.hpp"
#include "../../lang/obj.hpp"
#include "../../lang/type.hpp"
#include "io/gpio/gpio.hpp"
#include "io/i2c/i2c.hpp"
#include "../../lang/mmadt/mmadt.hpp"
#include "util/poll.hpp"
#ifdef ARDUINO
#include "io/pwm/pwm.hpp"
#include "sensor/aht10/aht10.hpp"
#include "sensor/mlx90614/mlx90614.hpp"
#include "ui/oled/oled.hpp"
#include "ui/rgbled/rgbled.hpp"
#endif
#define TOTAL_INSTRUCTIONS 100
#define FOS_URI "/fos"

namespace fhatos {
  using namespace mmadt;

  class fOS {
  public:
    static void *import_types() {
      Typer::singleton()->start_progress_bar(10);
      Typer::singleton()->save_type(
          CHAR_FURI,
          *__(*CHAR_FURI, *INT_FURI, *STR_FURI)->merge(jnt(2))->count()->is(*__()->eq(jnt(1))));
      Typer::singleton()->save_type(INT8_FURI, Obj::to_type(INT8_FURI));
      Typer::singleton()->save_type(
          UINT8_FURI,
          *__(*UINT8_FURI, *INT_FURI, *INT_FURI)->is(*__()->gte(jnt(0)))->is(*__()->lte(jnt(255))));
      Typer::singleton()->save_type(INT16_FURI, Obj::to_type(INT16_FURI));
      Typer::singleton()->save_type(INT32_FURI, Obj::to_type(INT32_FURI));
      Typer::singleton()->save_type(NAT_FURI, *__(*NAT_FURI, *INT_FURI, *INT_FURI)->is(*__()->gte(jnt(0))));
      Typer::singleton()->save_type(
          CELSIUS_FURI,
          *__(*CELSIUS_FURI, *REAL_FURI, *REAL_FURI)->is(*__()->gte(real(-273.15))));
      Typer::singleton()->save_type(
          PERCENT_FURI,
          *__(*PERCENT_FURI, *REAL_FURI, *REAL_FURI)->is(*__()->gte(real(0.0)))->is(*__()->lte(real(100.0))));
      Typer::singleton()->save_type(HEX_FURI, *__(*HEX_FURI, *URI_FURI, *URI_FURI)->is(dool(true)));
      Typer::singleton()->save_type(MILLISECOND_FURI, Obj::to_type(INT_FURI));
      Typer::singleton()->save_type(SECOND_FURI, Obj::to_type(INT_FURI));
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ycommon types!! loaded!g]!! \n",FOS_URI "/+"));
      return nullptr;
    }

    static void import_query_processor() {
      Typer::singleton()->start_progress_bar(14);
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ybase types!! loaded!g]!! \n",MMADT_SCHEME "/+"));
    }

    static void *import_io() {
      Typer::singleton()->start_progress_bar(6);
      GPIO::import();
      I2C::import();
#ifdef ARDUINO
      PWM::import();
#endif
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yio types!! loaded!g]!! \n",FOS_URI "/io/+"));
      return nullptr;
    }

    static void *import_sensor() {
      Typer::singleton()->start_progress_bar(3);
#ifdef ARDUINO
      AHT10::import();
      MLX90614::import();
#endif
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ysensor types!! loaded!g]!! \n",FOS_URI "/sensor/+"));
      return nullptr;
    }

    static void *import_ui() {
      Typer::singleton()->start_progress_bar(6);
#ifdef ARDUINO
      RGBLED::import();
      OLED::import();
#endif
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yui types!! loaded!g]!! \n",FOS_URI "/ui/+"));
      return nullptr;
    }

    static void *import_util() {
      Typer::singleton()->start_progress_bar(6);
      Poll::import();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yutil types!! loaded!g]!! \n",FOS_URI "/util/+"));
      return nullptr;
    }

  };
}
#endif
