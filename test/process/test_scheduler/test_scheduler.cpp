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

#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_FOS_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /scheduler/#
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_scheduler_config() {

  }

  void test_scheduler_spawn_destroy() {
    PROCESS("/scheduler/a -> |[:loop=>from(/scheduler/z,0).plus(1).to(/scheduler/z)]");
    FOS_TEST_REC_KEYS(PROCESS("*/scheduler/a"), {vri(":loop")});
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/z"));
    PROCESS("/sys/scheduler/:spawn(@/scheduler/a)");
    FOS_TEST_REC_KEYS(PROCESS("*/scheduler/a"), List<Obj_p>({vri(":loop"),vri(":delay"),vri(":yield"),vri(":stop")}));
    const Obj_p obj_z_1 = PROCESS("*/scheduler/z");
    FOS_TEST_OBJ_GT(obj_z_1, jnt(0));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const Obj_p obj_z_2 = PROCESS("*/scheduler/z");
    FOS_TEST_OBJ_GT(obj_z_2, obj_z_1);
    PROCESS("/scheduler/a/:stop()");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Router::singleton()->loop(); // TODO: why necessary?
    const Obj_p obj_z_3 = PROCESS("*/scheduler/z");
    FOS_TEST_OBJ_GT(obj_z_3, obj_z_2);
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:loop"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:delay"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:yield"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:stop"));
    const Obj_p obj_z_4 = PROCESS("*/scheduler/z");
    FOS_TEST_OBJ_EQUAL(obj_z_4, obj_z_3);
    PROCESS("/scheduler/a -> noobj");
    PROCESS("/scheduler/a/:loop -> noobj");
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:loop"));
   // TODO: source write  Router::singleton()->unsubscribe(*Router::singleton()->vid, "#");
  }

  void test_scheduler_spawn_destroy_for_mono() {
    PROCESS("/scheduler/a -> 0");
    PROCESS("/scheduler/a/:loop -> |(from(/scheduler/a,0).plus(1).to(/scheduler/a))");
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:loop"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("*/scheduler/a"));
    PROCESS("/sys/scheduler/:spawn(@/scheduler/a)");
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:delay"));
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:yield"));
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:stop"));
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:loop"));
    const Obj_p obj_z_1 = PROCESS("*/scheduler/a");
    FOS_TEST_OBJ_GT(obj_z_1, jnt(0));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const Obj_p obj_z_2 = PROCESS("*/scheduler/a");
    FOS_TEST_OBJ_GT(obj_z_2, obj_z_1);
   // Router::singleton()->loop(); // TODO: why necessary?
    PROCESS("/scheduler/a -> noobj");
  //  Router::singleton()->loop(); // TODO: why necessary?
    std::this_thread::sleep_for(std::chrono::seconds(1));
    FOS_TEST_OBJ_NTEQL(Obj::to_noobj(), PROCESS("*/scheduler/a/:loop"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:delay"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:yield"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a/:stop"));
//    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*/scheduler/a"));
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_scheduler_config); //
      // FOS_RUN_TEST(test_scheduler_spawn_destroy); //
      // FOS_RUN_TEST(test_scheduler_spawn_destroy_for_mono); //
      )

}

SETUP_AND_LOOP();
