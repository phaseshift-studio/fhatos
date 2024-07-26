#ifndef fhatos_test_scheduler_hpp
#define fhatos_test_scheduler_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>


namespace fhatos {
  void test_threads() {
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->find("/test/abc/thread-1")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler::singleton()->spawn(new Thread("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-1"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/#")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/#")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler::singleton()->spawn(new Thread("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/thread-2"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/thread-2")->size());
    TEST_ASSERT_EQUAL_INT(2, Scheduler::singleton()->find("/test/abc/+")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/+/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/+/thread-2")->size());
    ////////////////////////////////////////////////////////////////////////////
    Scheduler::singleton()->destroy("/test/abc/thread-1");
    Scheduler::singleton()->barrier("thread-1_dead",
                                    [] { return Scheduler::singleton()->count("/test/abc/thread-1") == 0; });
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/#"));
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->count("/test/abc/#"));
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->find("/test/abc/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/thread-2")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/abc/+")->size());
    TEST_ASSERT_EQUAL_INT(0, Scheduler::singleton()->find("/test/+/thread-1")->size());
    TEST_ASSERT_EQUAL_INT(1, Scheduler::singleton()->find("/test/+/thread-2")->size());
    Scheduler::singleton()->destroy("/test/abc/thread-2");
    Scheduler::singleton()->barrier("thread-2_dead", [] { return Scheduler::singleton()->count("/test/#") == 0; });
  }

  /* template<typename ROUTER>
   void test_bcode() {
     Scheduler<>::singleton()->publish(
         Scheduler<ROUTER>::singleton()->id().query("?spawn"),
         ptr<const Rec>(Rec({
                 {new Uri("id"), new Uri("test_spawn")},
                 {new Uri("setup"), new Bytecode(__(0).ref("loop").bcode->value())},
                 {new Uri("loop"),
                  new Bytecode(__(0)
                                   .dref("loop")
                                   .plus(1)
                                   .bswitch({{_.is(_.lt(0)), _.plus(0)},
                                             {_.is(_.gt(10)), _.mult(-1).ref("loop").publish(
                                                                  Scheduler<ROUTER>::singleton()->id().query("?destroy"),
                                                                  Uri("test_spawn"))},
                                             {_, _.ref("loop")}})
                                   .bcode->value())},

             })
             .as<Rec>("thread")));
     Scheduler<ROUTER>::singleton()->shutdown();
   }*/

  FOS_RUN_TESTS( //
      for (Router *router
           : List<Router *>{FOS_TEST_ROUTERS}) { //
        GLOBAL_OPTIONS->ROUTING = router; //
        router->clear(); //
        LOG(INFO, "!r!_Testing with %s!!\n", router->toString().c_str()); //
        FOS_RUN_TEST(test_threads); //
        //  FOS_RUN_TEST(test_bcode<LocalRouter>); //
      });

} // namespace fhatos

SETUP_AND_LOOP();

#endif
