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
#include "io/gpio/gpio.hpp"
#include "io/i2c/i2c.hpp"
#include "net/inet.hpp"
#include "s/dsm.hpp"
#include "s/fs/fs.hpp"
#include "s/heap.hpp"
#include "sys/router/memory/memory.hpp"
#include "sys/scheduler/thread/thread.hpp"
#include "sys/typer/typer.hpp"
#include "ui/button/button.hpp"
#include "ui/console.hpp"
#include "util/llm/ollama_server.hpp"
#include "util/log.hpp"
#include "util/poll.hpp"
#include "util/text.hpp"
#include "util/time.hpp"
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
    static void register_module() {
      modules_fos_q();
      //QSub::register_module();
      modules_fos_io();
      modules_fos_ui();
      // import_io(patterns);
      //  import_sys(patterns);
      // import_sensor(patterns);
      // import_ui({});
      // import_util({});
#ifdef ESP_PLATFORM
      WIFIx::register_module();
#endif
      Inet::register_module();
      Time::register_module();
      Thread::register_module();
      Heap<>::register_module();
      DSM::register_module();
      FS::register_module();
      Memory::register_module();
      GPIO::register_module();
#ifdef NATIVE
      OllamaServer::register_module();
#endif
    }

    static void modules_fos_ui() {
      REGISTERED_MODULES->insert_or_assign("/fos/ui",
                                           InstBuilder::build(Typer::singleton()->vid->add_component("/fos/ui"))
                                               ->domain_range(NOOBJ_FURI, {0, 0}, REC_FURI, {1, 1})
                                               ->inst_f([](const Obj_p &, const InstArgs &) {
                                                 Button::import();
                                                 Terminal::import();
                                                 Console::import();
#ifdef ARDUINO
                                                 RGBLED::import();
                                                 OLED::register_module();
#endif
                                                 return Obj::to_rec();
                                               })
                                               ->create());
    }

    static void modules_fos_q() {
      REGISTERED_MODULES->insert_or_assign(
          FOS_URI "/q",
          InstBuilder::build(Typer::singleton()->vid->add_component(FOS_URI "/q"))
              ->domain_range(NOOBJ_FURI, {0, 0}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec({{vri(FOS_URI "/q/sub/msg"), Obj::to_rec({{"target", Obj::to_type(URI_FURI)},
                                                                             {"payload", Obj::to_bcode()},
                                                                             {"retain", Obj::to_type(BOOL_FURI)}})},
                                    {vri(FOS_URI "/q/sub/sub"), Obj::to_rec({{"source", Obj::to_type(URI_FURI)},
                                                                             {"pattern", Obj::to_type(URI_FURI)},
                                                                             {"on_recv", Obj::to_bcode()}})},
                                    {vri(FOS_URI "/q"), Obj::to_rec()},
                                    {vri(FOS_URI "/q/sub"), Obj::to_rec()},
                                    {vri(FOS_URI "/q/doc"), Obj::to_rec()}});
              })
              ->create());
    }

    static void modules_fos_io() {
      //     I2C::import();
#ifdef ARDUINO
      //    PWM::import();
      //    WIFIx::import();
      //     OTA::import();
#endif
    }

    /* static void *import_io(const std::vector<fURI> &patterns = {}) {
       Typer::singleton()->start_progress_bar(6);
       Typer::singleton()->set_filters(const_cast<std::vector<fURI> *>(&patterns));

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

       Typer::singleton()->clear_filters();
       Typer::singleton()->end_progress_bar(
           StringHelper::format("\n\t\t!^u1^ !g[!b%s !ysys types!! imported!g]!! \n", FOS_URI "/sys/+"));
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
     }*/
  };
} // namespace fhatos
#endif
