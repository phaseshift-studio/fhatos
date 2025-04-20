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
#ifndef fhatos_fos_obj_hpp
#define fhatos_fos_obj_hpp

#include "../../fhatos.hpp"
#include "../../lang/obj.hpp"
#include "../../lang/type.hpp"
#include "io/gpio/gpio.hpp"
#include "io/i2c/i2c.hpp"
#include "../../lang/mmadt/mmadt.hpp"
#include "util/poll.hpp"
#include "util/text.hpp"
#include "sys/scheduler/thread/thread.hpp"
#include "sys/memory/memory.hpp"
#include "../../lang/processor/processor.hpp"
#include "ui/console.hpp"
#include "ui/button/button.hpp"
#ifdef ARDUINO
#include "net/wifi.hpp"
#include "net/ota.hpp"
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
      load_processor();
      Typer::singleton()->start_progress_bar(10);
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->save_type(
          *MESSAGE_FURI, Obj::to_rec({
              {"target", Obj::to_type(URI_FURI)},
              {"payload", Obj::to_bcode()},
              {"retain", Obj::to_type(BOOL_FURI)}}));
      Typer::singleton()->save_type(
          *SUBSCRIPTION_FURI, Obj::to_rec({
              {"source", Obj::to_type(URI_FURI)},
              {"pattern", Obj::to_type(URI_FURI)},
              {"on_recv", Obj::to_bcode()}}));
      Typer::singleton()->save_type(*Q_PROC_FURI, Obj::to_rec());

      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ycommon types!! loaded!g]!! \n",FOS_URI "/+"));
      return nullptr;
    }

    static void *import_q_procs() {
      Typer::singleton()->start_progress_bar(14);
      Typer::singleton()->save_type(Q_PROC_FURI->extend("sub"), Obj::to_rec());
      Typer::singleton()->save_type(Q_PROC_FURI->extend("doc"), Obj::to_rec());
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yquery processors!! loaded!g]!! \n",FOS_URI "/q/+"));
      return nullptr;
    }

    static void *import_io() {
      Typer::singleton()->start_progress_bar(6);
      GPIO::import();
      I2C::import();
#ifdef ARDUINO
      PWM::import();
      WIFIx::import();
      OTA::import();
#endif
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yio types!! loaded!g]!! \n",FOS_URI "/io/+"));
      return nullptr;
    }

    static void *import_sys() {
      Typer::singleton()->start_progress_bar(2);
      Memory::import();
      Thread::import();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ysys types!! loaded!g]!! \n",FOS_URI "/sys/+"));
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
      Button::import();
      Terminal::import();
      Console::import();
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
      Log::import();
      Poll::import();
      Text::import();

      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yutil types!! loaded!g]!! \n",FOS_URI "/util/+"));
      return nullptr;
    }
  };
}
#endif
