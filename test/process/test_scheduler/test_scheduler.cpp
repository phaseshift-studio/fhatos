#ifndef fhatos_test_scheduler_hpp
#define fhatos_test_scheduler_hpp

#include <test_fhatos.hpp>
//
#include <process/actor/actor.hpp>
#include <process/router/local_router.hpp>
#include <structure/furi.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <language/fluent.hpp>


namespace fhatos {

  void test_schedule() {
    TEST_ASSERT_EQUAL_INT(0, Scheduler<>::singleton()->count("#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler<>::singleton()->find("abc/thread-1")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler<>::singleton()->spawn(new Thread("abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->count("#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("abc/thread-1")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler<>::singleton()->spawn(new Thread("abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(2, Scheduler<>::singleton()->count("#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("abc/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("abc/thread-2")->size());
    TEST_ASSERT_EQUAL_INT(2, Scheduler<>::singleton()->find("abc/+")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("+/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("+/thread-2")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler<>::singleton()->destroy("abc/thread-1");
    Scheduler<>::singleton()->barrier("thread-1_dead",
                                      [] { return Scheduler<>::singleton()->count("abc/thread-1") == 0; });
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->count("#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler<>::singleton()->find("abc/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("abc/thread-2")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("abc/+")->size());
    TEST_ASSERT_EQUAL_INT(0, Scheduler<>::singleton()->find("+/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler<>::singleton()->find("+/thread-2")->size());
    Scheduler<>::singleton()->destroy("abc/thread-2");
    Scheduler<>::singleton()->barrier("thread-2_dead");
  }


  FOS_RUN_TESTS( //
      Scheduler<>::singleton(); //
      LocalRouter::singleton(); //
      FOS_RUN_TEST(test_schedule); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
