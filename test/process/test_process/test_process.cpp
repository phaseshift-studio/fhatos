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

#ifndef fhatos_test_process_hpp
#define fhatos_test_process_hpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_SHARED_MEMORY /test/#
#define FOS_DEPLOY_EXT
#include <test_fhatos.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {

  void test_process(const PType ptype) {
    const char *pc = strdup(ProcessTypes.to_chars(ptype).c_str());
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/thread"));
    process("/test/thread -> "
            "block(thread[["
            ":setup=>map(/test/thread/a->345),"
            ":loop=>map(from(/test/thread/x,0).plus(1).to(/test/thread/x)),"
            ":stop=>map(/test/thread/b->57)]])");
    process("%s/:spawn --> @/test/thread", Scheduler::singleton()->vid()->toString().c_str());
    sleep(1);
    //TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/thread"));
    //const Obj_p b_1 = process("*/test/thread/b")->objs_value()->at(0);
    //TEST_ASSERT_TRUE(b_1->is_noobj());
    const Int_p x_1 = process("*/test/thread/x")->objs_value()->at(0);
    FOS_TEST_OBJ_GT(x_1, jnt(0))
    const Int_p x_2 = process("*/test/thread/x")->objs_value()->at(0);
    FOS_TEST_OBJ_GT(x_2, jnt(0));
    FOS_TEST_OBJ_GT(x_2, x_1);
    FOS_TEST_OBJ_EQUAL(jnt(345), process("*/test/thread/a")->objs_value()->at(0));
    process("/test/thread -> noobj");
    sleep(1);
    FOS_TEST_OBJ_EQUAL(jnt(57), process("*/test/thread/b")->objs_value()->at(0));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/thread"));
    //FOS_TEST_OBJ_EQUAL(noobj(),process("/test/%s/b -> noobj",pc)->objs_value()->at(0));
    free((void*)pc);
    sleep(1);
  }

  void test_thread() {
    test_process(PType::THREAD);
  }

  void test_fiber() {
    test_process(PType::FIBER);
  }

  void test_coroutine() {
    test_process(PType::COROUTINE);
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_thread); //
      //FOS_RUN_TEST(test_fiber); //
      //FOS_RUN_TEST(test_coroutine); //
      );

} // namespace fhatos

SETUP_AND_LOOP();

#endif