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

#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_MMADT_EXT_TYPE
#define FOS_DEPLOY_FOS_TYPE
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#include "../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_thread_creation() {
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>1,halt=>true]]"), PROCESS("thread[[loop=>1,halt=>true]]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>2,halt=>true]]"), PROCESS("thread[[loop=>2]]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>3,halt=>false]]"), PROCESS("thread[[loop=>3,halt=>false]]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>4,halt=>true]]"), PROCESS("thread[[loop=>4]]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>noobj,halt=>true]]"), PROCESS("thread[[=>]]"));
  }

  void test_thread_spawn() {
    const Rec_p thread = PROCESS("thread[[loop=>^(from(|abc,0).plus(1).to(abc))]]@z");
    LOG_WRITE(INFO, thread.get(), L("testing {}\n", thread->toString()));
    FOS_TEST_FURI_EQUAL(*THREAD_FURI, PROCESS("*z.type()")->uri_value());
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(true), thread->obj_get("halt")); // ensure created thread starts off halted
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("*abc")); // should be noobj else loop() is being pre-executed (i.e. block failing)
    int counter = 0;
    PROCESS("/sys/scheduler::spawn(@z)"); // prev: test thread structure pre-spawn;
    // post: test thread structure post-spawn
    Thread::delay(50); // give thread time to execute for a while
    FOS_TEST_OBJ_EQUAL(Obj::to_lst({vri("z")}),Scheduler::singleton()->obj_get("spawn")); // ensure scheduler posted
    FOS_TEST_FURI_EQUAL(*INT_FURI, PROCESS("*abc.type()")->uri_value());
    FOS_TEST_OBJ_NOT_EQUAL(Obj::to_noobj(), thread);
    TEST_ASSERT_FALSE(thread->obj_get("halt")->bool_value()); // ensure spawned thread isn't halted
    TEST_ASSERT_FALSE(PROCESS("*z/halt")->bool_value()); // ensure spawned thread isn't halted
    counter = PROCESS("*abc")->int_value();
    TEST_ASSERT_GREATER_THAN_INT(0, counter);
    for(int i = 0; i < 50; i++) {
      Thread::delay(10);
      TEST_ASSERT_GREATER_THAN_INT(counter, PROCESS("*abc")->int_value());
      counter = PROCESS("*abc")->int_value();
    }
    PROCESS("z/halt -> true");
    TEST_ASSERT_TRUE(PROCESS("*abc")->is_int());
    counter = PROCESS("*abc")->int_value();
    while(/*!thread->obj_get("halt")->bool_value() &&*/
          !Scheduler::singleton()->obj_get("spawn")->lst_value()->empty()) {
      Thread::delay(10);
      Router::singleton()->loop();
    }
    //FOS_TEST_OBJ_EQUAL(Obj::to_lst(), Scheduler::singleton()->obj_get("spawn")); // ensure scheduler removed thread furi
    //TEST_ASSERT_EQUAL(counter,PROCESS("*abc")->int_value()); // make sure thread isn't continuing after halting
    // FOS_TEST_OBJ_EQUAL(thread,PROCESS("*z")); // TODO: uncomment when spawn halts at the obj
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_thread_creation); //
      FOS_RUN_TEST(test_thread_spawn); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
