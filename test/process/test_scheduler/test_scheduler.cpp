#ifndef fhatos_test_scheduler_hpp
#define fhatos_test_scheduler_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>


namespace fhatos {
  void test_threads() {
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/abc/thread-1"));
    ////////////////////////////////////////////////////////////////////////////
    Thread *a = new Thread("/test/abc/thread-1");
    Scheduler::singleton()->spawn(a);
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    ////////////////////////////////////////////////////////////////////////////
    Thread *b = new Thread("/test/abc/thread-2");
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
    Scheduler::singleton()->kill("/test/abc/thread-1");
    Scheduler::singleton()->barrier("thread-1_dead",
                                    [] { return Scheduler::singleton()->count("/test/abc/thread-1") == 0; });
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/+"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/+/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/+/thread-2"));
    Scheduler::singleton()->kill("/test/abc/thread-2");
    Scheduler::singleton()->barrier("thread-2_dead", [] { return Scheduler::singleton()->count("/test/#") == 0; });
    delete a;
    delete b;
  }

  /* template<typename ROUTER>
   void test_bcode() {
     Scheduler<>::singleton()->publish(
         Scheduler<ROUTER>::singleton()->id().query("?spawn"),
         ptr<const Rec>(Rec({
                 {new Uri("id"), new Uri("test_spawn")},
                 {new Uri("setup"), new Bytecode(__(0).ref("loop")._bcode->value())},
                 {new Uri("loop"),
                  new Bytecode(__(0)
                                   .dref("loop")
                                   .plus(1)
                                   .bswitch({{_.is(_.lt(0)), _.plus(0)},
                                             {_.is(_.gt(10)), _.mult(-1).ref("loop").publish(
                                                                  Scheduler<ROUTER>::singleton()->id().query("?kill"),
                                                                  Uri("test_spawn"))},
                                             {_, _.ref("loop")}})
                                   ._bcode->value())},

             })
             .as<Rec>("thread")));
     Scheduler<ROUTER>::singleton()->shutdown();
   }*/

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_threads); //
      //  FOS_RUN_TEST(test_bcode<LocalRouter>); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
