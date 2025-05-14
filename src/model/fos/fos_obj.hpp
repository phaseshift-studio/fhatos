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
#include "../../lang/mmadt/mmadt.hpp"
#include "../../lang/obj.hpp"
#include "../../lang/processor/processor.hpp"
#include "../../lang/type.hpp"
#include "io/fs/fs.hpp"
#include "io/gpio/gpio.hpp"
#include "io/i2c/i2c.hpp"
#include "sys/memory/memory.hpp"
#include "sys/router/structure/dsm.hpp"
#include "sys/router/structure/heap.hpp"
#include "sys/scheduler/thread/thread.hpp"
#include "ui/button/button.hpp"
#include "ui/console.hpp"
#include "util/poll.hpp"
#include "util/text.hpp"
#ifdef ARDUINO
#include "io/pwm/pwm.hpp"
#include "net/ota.hpp"
#include "net/wifi.hpp"
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
    static void *import(const std::vector<fURI> &patterns) {
      modules_fos_qproc();
      modules_fos_io();
      import_q_proc(patterns);
      import_io(patterns);
      import_sys(patterns);
      import_structure(patterns);
      import_sensor(patterns);
      import_ui(patterns);
      import_util(patterns);
      return nullptr;
    }

    static void modules_fos_qproc() {
      InstBuilder::build(Typer::singleton()->vid->extend("module/fos/qproc"))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return Obj::to_rec({{vri(MESSAGE_FURI), Obj::to_rec({{"target", Obj::to_type(URI_FURI)},
                                                                 {"payload", Obj::to_bcode()},
                                                                 {"retain", Obj::to_type(BOOL_FURI)}})},
                                {vri(SUBSCRIPTION_FURI), Obj::to_rec({{"source", Obj::to_type(URI_FURI)},
                                                                      {"pattern", Obj::to_type(URI_FURI)},
                                                                      {"on_recv", Obj::to_bcode()}})},
                                {vri(Q_PROC_FURI), Obj::to_rec()},
                                {vri(Q_PROC_FURI->extend("sub")), Obj::to_rec()},
                                {vri(Q_PROC_FURI->extend("doc")), Obj::to_rec()}});
          })
          ->save();
    }

    static void modules_fos_io() {
      GPIO::load_module();
      //     I2C::import();
#ifdef ARDUINO
      //    PWM::import();
      //    WIFIx::import();
      //     OTA::import();
#endif
    }

    static void *import_q_proc(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(5);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
      Typer::singleton()->save_type(*MESSAGE_FURI, Obj::to_rec({{"target", Obj::to_type(URI_FURI)},
                                                                {"payload", Obj::to_bcode()},
                                                                {"retain", Obj::to_type(BOOL_FURI)}}));
      Typer::singleton()->save_type(*SUBSCRIPTION_FURI, Obj::to_rec({{"source", Obj::to_type(URI_FURI)},
                                                                     {"pattern", Obj::to_type(URI_FURI)},
                                                                     {"on_recv", Obj::to_bcode()}}));
      Typer::singleton()->save_type(*Q_PROC_FURI, Obj::to_rec());
      Typer::singleton()->save_type(Q_PROC_FURI->extend("sub"), Obj::to_rec());
      Typer::singleton()->save_type(Q_PROC_FURI->extend("doc"), Obj::to_rec());
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yquery types!! imported!g]!! \n", FOS_URI "/q/+"));
      return nullptr;
    }

    static void *import_io(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(6);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
       GPIO::import();
      I2C::import();
#ifdef ARDUINO
      PWM::import();
      WIFIx::import();
      OTA::import();
#endif
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yio types!! imported!g]!! \n", FOS_URI "/io/+"));
      return nullptr;
    }

    static void *import_sys(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(2);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
      Memory::import();
      Thread::import();
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ysys types!! imported!g]!! \n", FOS_URI "/sys/+"));
      return nullptr;
    }

    static void *import_structure(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(3);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
      Heap<>::import();
      DSM::import();
      FS::import();
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ystructure types!! loaded!g]!! \n", FOS_URI "/s/+"));
      return nullptr;
    }

    static void *import_sensor(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(3);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
#ifdef ARDUINO
      AHT10::import();
      MLX90614::import();
#endif
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ysensor types!! loaded!g]!! \n", FOS_URI "/sensor/+"));
      return nullptr;
    }

    static void *import_ui(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(6);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
      Button::import();
      Terminal::import();
      Console::import();
#ifdef ARDUINO
      RGBLED::import();
      OLED::import();
#endif
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yui types!! loaded!g]!! \n", FOS_URI "/ui/+"));
      return nullptr;
    }

    static void *import_util(const std::vector<fURI> &patterns = {}) {
      Typer::singleton()->start_progress_bar(3);
      Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));
      Log::import();
      Poll::import();
      Text::import();
      Typer::singleton()->clear_filters();
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yutil types!! loaded!g]!! \n", FOS_URI "/util/+"));
      return nullptr;
    }
  };
} // namespace fhatos
#endif
