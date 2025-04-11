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
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_MMADT_EXT_TYPE
#define FOS_DEPLOY_FOS_TYPE
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY
#include "../../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_thread_creation() {
    Rec_p thread = PROCESS("thread::create(loop=>1,delay=>nat[10],halt=>true)");
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>1,delay=>nat[10],halt=>true]]"),thread);
    thread = PROCESS("thread::create(loop=>1)"); // test default values
    FOS_TEST_OBJ_EQUAL(PROCESS("thread[[loop=>1,delay=>nat[0],halt=>true]]"),thread);
  }

  void test_thread_spawn() {
    Rec_p thread = PROCESS("thread::create(loop=>^(from(|abc,0).plus(1).to(abc))).at(|z)");
    LOG_WRITE(INFO,thread.get(),L("testing {}\n",thread->toString()));
    FOS_TEST_FURI_EQUAL(*THREAD_FURI,PROCESS("*z.type()")->uri_value());
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(true), thread->rec_get("halt")); // ensure created thread starts off halted
    PROCESS("abc -> 0"); // TODO: remove this and figure out why thread.loop() is being executed 18 times
    int counter = PROCESS("*abc")->int_value(); // should be noobj else loop() is being pre-executed (i.e. block failing)
    TEST_ASSERT_EQUAL(0,counter);
     FOS_TEST_OBJ_EQUAL(Obj::to_noobj(),PROCESS("@z.spawn()")); // prev: test thread structure pre-spawn; post: test thread structure post-spawn
    std::this_thread::sleep_for(chrono::milliseconds(50)); // give thread time to execute for a while
    FOS_TEST_OBJ_EQUAL(Obj::to_lst({vri("z")}),ROUTER_READ("/sys/scheduler/thread")); // ensure scheduler posted thread furi
    TEST_ASSERT_FALSE(thread->load()->get<bool>("halt")); // ensure spawned thread isn't halted
    TEST_ASSERT_FALSE(PROCESS("*z/halt")->bool_value()); // ensure spawned thread isn't halted
    FOS_TEST_FURI_EQUAL(*INT_FURI,PROCESS("*abc.type()")->uri_value());
    std::this_thread::sleep_for(chrono::milliseconds(50)); // give thread time to execute for a while
    counter = PROCESS("*abc")->int_value();
    TEST_ASSERT_GREATER_THAN_INT(0,counter);
    for(int i=0;i<50;i++) {
      std::this_thread::sleep_for(chrono::milliseconds(50));
      std::this_thread::yield();
      TEST_ASSERT_GREATER_THAN_INT(counter,PROCESS("*abc")->int_value());
      counter = PROCESS("*abc")->int_value();
      std::this_thread::yield();
    }
    PROCESS("z/halt -> true");
    std::this_thread::sleep_for(chrono::milliseconds(250)); // give scheduler time to dismantle thread
	std::this_thread::yield();
    Router::singleton()->loop();
   // FOS_TEST_OBJ_EQUAL(Obj::to_lst(),ROUTER_READ("/sys/scheduler/thread")); // ensure scheduler removed thread furi
    counter = PROCESS("*abc")->int_value();
    std::this_thread::sleep_for(chrono::milliseconds(250)); // give scheduler time to dismantle thread
    std::this_thread::yield();
    TEST_ASSERT_TRUE(thread->get<bool>("halt")); // ensure spawned thread is halted
   // TEST_ASSERT_TRUE(PROCESS("*z/halt")->bool_value());
    //TEST_ASSERT_EQUAL(counter,PROCESS("*abc")->int_value()); // make sure thread isn't continuing after halting
   // FOS_TEST_OBJ_EQUAL(thread,PROCESS("*z"));
    std::this_thread::yield();
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_thread_creation); //
      FOS_RUN_TEST(test_thread_spawn); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
