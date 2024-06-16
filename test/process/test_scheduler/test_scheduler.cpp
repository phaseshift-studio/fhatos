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
  void test_threads() {
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

  template<typename ROUTER>
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
  }

  FOS_RUN_TESTS( //
      Scheduler<FOS_DEFAULT_ROUTER>::singleton(); //
      LocalRouter::singleton(); //
      FOS_RUN_TEST(test_threads); //
      FOS_RUN_TEST(test_bcode<LocalRouter>); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
