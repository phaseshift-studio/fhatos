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

#ifndef fhatos_test_scheduler_hpp
#define fhatos_test_scheduler_hpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_SHARED_MEMORY /test/#
#include <test_fhatos.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  void test_threads() {
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/abc/thread-1"));
    ////////////////////////////////////////////////////////////////////////////
    const auto a = std::make_shared<Thread>("/test/abc/thread-1");
    Scheduler::singleton()->spawn(a);
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    ////////////////////////////////////////////////////////////////////////////
    const auto b = std::make_shared<Thread>("/test/abc/thread-2");
    Scheduler::singleton()->spawn(b);
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->count("/test/abc/+"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/+/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/+/thread-2"));
    ////////////////////////////////////////////////////////////////////////////
    a->stop();
    //Scheduler::singleton()->kill("/test/abc/thread-1");
    Scheduler::singleton()->barrier("thread-1_dead",
                                    [] { return Scheduler::singleton()->count("/test/abc/thread-1") == 0; });
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/+"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/+/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/+/thread-2"));
    b->stop();
    //Scheduler::singleton()->kill("/test/abc/thread-2");
    Scheduler::singleton()->barrier("thread-2_dead", [] { return Scheduler::singleton()->count("/test/#") == 0; });
    Scheduler::singleton()->stop();
    Scheduler::singleton()->barrier("complete");
  }

  FOS_RUN_TESTS( //
          FOS_RUN_TEST(test_threads); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
