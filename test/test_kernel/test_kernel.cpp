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

#ifndef fhatos_test_kernel_cpp
#define fhatos_test_kernel_cpp

#include "../../src/furi.hpp"
#include "../test_fhatos.hpp"
#include "../../src/kernel.hpp"
#include "../../src/lang/mmadt/parser.hpp"
#include "../../src/lang/mmadt/mmadt_obj.hpp"
#include "../../src/fhatos.hpp"
#include "../../src/kernel.hpp"
#include "../../src/structure/router.hpp"
#include "../../src/util/argv_parser.hpp"
#include "../../src/model/fos/sys/scheduler/scheduler.hpp"
#include "../../src/lang/type.hpp"
#include "../../src/lang/mmadt/parser.hpp"
#include "../../src/model/fos/ui/console.hpp"
#include "../../src/model/fos/ui/terminal.hpp"
#include "../../src/model/fos/util/log.hpp"
//#include FOS_FILE_SYSTEM(fs.hpp)
#include "../../src/structure/stype/mqtt/mqtt.hpp"
#include "../../src/structure/stype/heap.hpp"
///////////// COMMON MODELS /////////////
//#include <model/driver/gpio/arduino_gpio_driver.hpp>
//#include <model/driver/i2c/arduino_i2c_master_driver.hpp>
// #include <model/pin/gpio.hpp>
// #include <model/pin/interrupt.hpp>
// #include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#include "../../src/util/esp/psram_allocator.hpp"
#include "../../src/model/soc/esp/wifi.hpp"
#include "../../src/model/soc/memory/esp32/memory.hpp"
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT string(getenv("FHATOS_HOME")).append("/data").c_str()
#define ALLOC
#else
#define FOS_FS_MOUNT "/"
#define ALLOC PSRAMAllocator<Pair<const ID_p, Obj_p>>
#endif


namespace fhatos {

  void test_basic_kernel() {
    load_processor(); // TODO: remove
    const ptr<Kernel> kp = Kernel::build()
        ->using_printer(Ansi<>::singleton())
        ->with_ansi_color(true)
        ->with_log_level(LOG_TYPES.to_enum("INFO"))
        ->display_splash(ANSI_ART)
        ->display_architecture()
        ->display_reset_reason()
        ->display_note("Use !b" STR(FOS_NOOBJ_TOKEN) "!! for !rnoobj!!")
        ->display_note("!r.!go!bO !yloading !bsystem !yobjs!! !bO!go!r.!!")
        ->using_scheduler(Scheduler::singleton("/sys/scheduler"))
        ->using_router(Router::singleton("/sys/router"))
        ////////////////// SYS STRUCTURE
        ->mount(Heap<ALLOC>::create("/boot/#"))
        ->mount(Heap<ALLOC>::create("/sys/#"))
        ->import(Heap<>::import("/sys/lib/heap"))
        ->using_boot_config()
        ->import(Router::import())
        ->drop_config("router")
        ->import(Scheduler::import())
        ////////////////// USER STRUCTURE(S)
        ->display_note("!r.!go!bO !yloading !blanguage !yobjs!! !bO!go!r.!!")
        ->mount(Structure::create<Heap<>>(MMADT_SCHEME "/#"))
        ->mount(Heap<>::create(FOS_URI "/#"))
        ->import(fOS::import_q_procs())
        ->import(mmadt::mmADT::import())
        ->display_note("!r.!go!bO !yloading !bio !yobjs!! !bO!go!r.!!")
        ->mount(Structure::create<Heap<>>("/io/#"))
        ->import(Log::import())
        //->import(Console::import("/io/lib/console"))
        ->install(Terminal::singleton("/io/terminal"))
        ->install(Log::create("/io/log",
                              Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/log")
                              ->or_else(Obj::to_rec({
                                  {"INFO", lst({vri("#")})},
                                  {"ERROR", lst({vri("#")})},
                                  {"WARN", lst()},
                                  {"DEBUG", lst()},
                                  {"TRACE", lst()}}))))
        ->install(mmadt::Parser::singleton("/io/parser"));
    //->import(mmadt::Parser::import("/io/lib/parser"))
    //->mount(Heap<>::create("+/#"))
    // TODO: ->with_bcode(OBJ_PARSER(string(
    //            "print('!r.!go!bO !yloading !buser !yobjs!! !bO!go!r.!!');"
    //          "|<+/#>./fos/lib/heap/create(_);")));

    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), Router::singleton()->read(ID(FOS_BOOT_CONFIG_VALUE_ID)));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), Router::singleton()->read(ID(FOS_BOOT_CONFIG_VALUE_ID).extend("router")));
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_basic_kernel); //
      )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
