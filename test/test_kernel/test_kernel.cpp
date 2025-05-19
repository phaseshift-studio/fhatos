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

#include "../../src/fhatos.hpp"
#include "../../src/furi.hpp"
#include "../../src/kernel.hpp"
#include "../../src/lang/mmadt/mmadt_obj.hpp"
#include "../../src/lang/mmadt/parser.hpp"
#include "../../src/model/fos/sys/router/router.hpp"
#include "../../src/model/fos/sys/router/structure/heap.hpp"
#include "../../src/model/fos/sys/scheduler/scheduler.hpp"
#include "../../src/model/fos/util/log.hpp"
#include "../../src/util/argv_parser.hpp"
#include "../test_fhatos.hpp"
///////////// COMMON MODELS /////////////
// #include <model/driver/gpio/arduino_gpio_driver.hpp>
// #include <model/driver/i2c/arduino_i2c_master_driver.hpp>
//  #include <model/pin/gpio.hpp>
//  #include <model/pin/interrupt.hpp>
//  #include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#include "../../src/model/soc/esp/wifi.hpp"
#include "../../src/model/soc/memory/esp32/memory.hpp"
#include "../../src/util/esp/psram_allocator.hpp"
#endif

#ifdef NATIVE
#define FOS_FS_MOUNT string(getenv("FHATOS_HOME")).append("/data").c_str()
#define ALLOC
#else
#define FOS_FS_MOUNT "/"
#define ALLOC PSRAMAllocator<Pair<const ID_p, Obj_p>>
#endif


namespace fhatos {

  void test_user_boot_loader() {}

  void test_basic_kernel() {
    // load_processor(); // TODO: remove
    BOOTING = true;
    Kernel::build()
        ->using_printer(Ansi<>::singleton())
        ->with_ansi_color(true)
        ->with_log_level(INFO)
        ->display_splash(ANSI_ART)
        ->display_architecture()
        ->display_reset_reason()
        ->display_note("!yloading !bsystem !yobjs!!")
        ->mount(Heap<>::create("/sys/#"))
        ->mount(Heap<>::create("/mnt/#", id_p("/mnt/mnt")))
        ->mount(Heap<>::create("/boot/#", id_p("/mnt/boot")))
        ->using_boot_config("/../test/data/boot/test_boot_config.obj")
        ->mount(Heap<>::create("/fos/#", id_p("/mnt/fos")))
        ->mount(Heap<>::create("/mmadt/#", id_p("/mnt/mmadt")))
        ////////////////// SYS STRUCTURE
        ->using_boot_config("/boot/boot_config.obj")
        ->display_note("!yloading !bsystem !yobjs!!")
        ->using_router("router")
        ->drop_config("router")
        ->using_scheduler("scheduler")
        ->drop_config("scheduler")
        ->using_typer("typer")
        ->drop_config("typer")
        ////////////////// USER STRUCTURE(S)
        ->display_note("!yloading !blanguage !yobjs!!")
        ->display_note("!yloading !bio !yobjs!!")
        ->mount(Structure::create<Heap<>>("/io/#"))
        ->install(make_shared<Log>(Obj::to_rec(rmap({{"config", Router::singleton()
                                                                    ->read(FOS_BOOT_CONFIG_VALUE_ID "/log")
                                                                    ->or_else(Obj::to_rec({{"INFO", lst({vri("#")})},
                                                                                           {"ERROR", lst({vri("#")})},
                                                                                           {"WARN", lst()},
                                                                                           {"DEBUG", lst()},
                                                                                           {"TRACE", lst()}}))}}),
                                               LOG_FURI, id_p("/io/log"))))
        ->install(mmadt::Parser::singleton("/io/parser"))
        ->drop_config("console");
    // ->loop();
    // Router::singleton()->loop();
    // Scheduler::singleton()->loop();
    BOOTING = false;
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), Router::singleton()->read(ID(FOS_BOOT_CONFIG_VALUE_ID)));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), Router::singleton()->read(ID(FOS_BOOT_CONFIG_VALUE_ID).extend("router")));
    /* FOS_TEST_OBJ_EQUAL(Obj::to_int(1),
                        jnt(Scheduler::singleton()->obj_get("spawn")->or_else(lst())->lst_value()->size()));
     FOS_TEST_OBJ_EQUAL(Obj::to_uri("/io/console"), Scheduler::singleton()->obj_get("spawn")->lst_value()->at(0));*/
    FOS_TEST_OBJ_EQUAL(Obj::to_int(0),
                       jnt(Scheduler::singleton()->obj_get("bundle")->or_else(lst())->lst_value()->size()));
    // ROUTER_WRITE("/io/console/halt", dool(true), true);
    ROUTER_WRITE(SCHEDULER_ID->extend("halt"), dool(true), true);
    Router::singleton()->loop();
    Scheduler::singleton()->loop();
    Scheduler::singleton()->stop();
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_user_boot_loader); //
      FOS_RUN_TEST(test_basic_kernel); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
