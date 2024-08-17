#ifndef fhatos_test_scheduler_hpp
#define fhatos_test_scheduler_hpp

#undef FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include FOS_PROCESS(thread.hpp)


namespace fhatos {
  void test_threads() {
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/abc/thread-1"));
    ////////////////////////////////////////////////////////////////////////////
    ptr<Thread> a = ptr<Thread>(new Thread("/test/abc/thread-1"));
    Scheduler::singleton()->spawn(a);
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    ////////////////////////////////////////////////////////////////////////////
    ptr<Thread> b = ptr<Thread>(new Thread("/test/abc/thread-2"));
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
