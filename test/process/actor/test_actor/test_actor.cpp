#ifndef fhatos_test_actor_hpp
#define fhatos_test_actor_hpp

#include <test_fhatos.hpp>
//
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <process/router/local_router.hpp>
//#include <process/router/meta_router.hpp>
#include <process/router/router.hpp>
#include <atomic>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  template<typename ROUTER>
  void test_actor_throughput() {
    std::atomic<int> *counter1 = new std::atomic<int>(0);
    std::atomic<int> *counter2 = new std::atomic<int>(0);
    Actor<Thread, ROUTER> *actor1 = new Actor<Thread, ROUTER>(
        ID("actor1@127.0.0.1"), [counter1](Actor<Thread, ROUTER> *self) {
          self->subscribe(ID("actor1@"), [counter1, self](const Message &message) {
            self->publish(ID("actor2@"), counter1->load(), TRANSIENT_MESSAGE);
            if (counter1->fetch_add(1) > 198)
              self->stop();
            // TEST_ASSERT_EQUAL(counter1->first, counter1->second);
          });
        });
    Actor<Thread, ROUTER> *actor2 = new Actor<Thread, ROUTER>(
        ID("actor2@127.0.0.1"), [counter2](Actor<Thread, ROUTER> *self) {
          self->subscribe(ID("actor2@"), [self, counter2](const Message &message) {
            FOS_TEST_ASSERT_EQUAL_FURI(ID("actor1@127.0.0.1"), message.source);
            FOS_TEST_ASSERT_EQUAL_FURI(self->id(), message.target);
            self->publish(ID("actor1@"), counter2->load(), TRANSIENT_MESSAGE);
            if (counter2->fetch_add(1) > 198)
              self->stop();
          });
        });
    Scheduler::singleton()->spawn(actor1);
    Scheduler::singleton()->spawn(actor2);
    actor1->publish("actor2@", "START!", TRANSIENT_MESSAGE);
    while (Scheduler::singleton()->count("actor1@127.0.0.1") ||
           Scheduler::singleton()->count("actor2@127.0.0.1")) {
    }
    ROUTER::singleton()->clear();
    TEST_ASSERT_EQUAL(counter1->load(), counter2->load());
    TEST_ASSERT_EQUAL(200, counter1->load());
    delete counter1;
    delete counter2;
  }

  template<typename ROUTER>
  void test_actor_by_router() {
    std::atomic<int> *counter1 = new std::atomic<int>(0);
    std::atomic<int> *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread, ROUTER>("actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread, ROUTER>("actor2@127.0.0.1");
    actor1->setup();
    actor2->setup();
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor1@127.0.0.1"), actor1->id());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor2@127.0.0.1"), actor2->id());
    RESPONSE_CODE rc =
        actor1->subscribe(
            actor1->id(), [actor1, actor2, counter1,counter2](const Message &message) {
              TEST_ASSERT_EQUAL_STRING("ping", message.payload->toString().c_str());
              FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
              FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
              TEST_ASSERT_EQUAL(
                  RESPONSE_CODE::OK,
                  actor1->publish(message.source, "pong", TRANSIENT_MESSAGE));
              counter1->fetch_add(1);
              counter2->fetch_add(1);
            });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
    rc = actor2->subscribe("actor2@127.0.0.1", [actor1, actor2,
                             counter2](const Message &message) {
                             TEST_ASSERT_EQUAL_STRING("pong", message.payload->toString().c_str());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor1->id());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor2->id());
                             counter2->fetch_add(1);
                           });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
    TEST_ASSERT_EQUAL(
        RESPONSE_CODE::REPEAT_SUBSCRIPTION,
        actor1->subscribe("actor1@127.0.0.1", [](const Message &message) {
          TEST_ASSERT_EQUAL_STRING("ping", message.payload->toString().c_str());
          }));

    actor2->publish(actor1->id(), "ping", TRANSIENT_MESSAGE);
    actor1->loop();
    actor2->loop();
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    ROUTER::singleton()->clear();
    delete counter1;
    delete counter2;
  }

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  template<typename ROUTER>
  void test_message_retain() {
    ROUTER::singleton()->clear();
    std::atomic<int> *counter1 = new std::atomic<int>(0);
    std::atomic<int> *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread, ROUTER>("actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread, ROUTER>("actor2@127.0.0.1");
    actor1->setup();
    actor2->setup();


    RESPONSE_CODE rc = actor1->subscribe(actor1->id(), [actor1, actor2,
                                           counter1](const Message &message) {
                                           TEST_ASSERT_TRUE(Str("ping") == message.payload->toStr());
                                           FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
                                           FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
                                           TEST_ASSERT_EQUAL_INT(0, counter1->load());
                                           counter1->fetch_add(1);
                                           TEST_ASSERT_EQUAL_INT(1, counter1->load());
                                         });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, rc);
    actor2->publish(actor1->id(), "ping", RETAIN_MESSAGE);
    actor1->loop();
    actor2->loop();
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(0, counter2->load());
    rc = actor2->subscribe("actor1@127.0.0.1", [actor1, actor2,
                             counter2](const Message &message) {
                             TEST_ASSERT_EQUAL_STRING("ping", message.payload->toString().c_str());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
                             counter2->fetch_add(1);
                           });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(1, counter2->load());
    //  TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe(actor1->id()));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribeSource());
    rc = actor1->subscribe("actor1@127.0.0.1", [actor1, actor2,
                             counter2](const Message &message) {
                             TEST_ASSERT_TRUE(Str("ping") == message.payload->toStr());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
                             FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
                             counter2->fetch_add(1);
                           });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(1, counter2->load());
    actor1->loop();
    actor2->loop();

    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    actor1->stop();
    actor2->stop();
    delete counter1;
    delete counter2;
    ROUTER::singleton()->clear();
  }

  template<typename ROUTER>
  void test_actor_serialization() {
    Actor<Thread, ROUTER> *actor = new Actor<Thread, ROUTER>("abc");
    const Pair<fbyte *, uint> buffer = actor->serialize();
    Actor<Thread, ROUTER> *clone =
        Actor<Thread, ROUTER>::deserialize(buffer.first);
    FOS_TEST_ASSERT_EQUAL_FURI(actor->id(), clone->id());
    FOS_TEST_PRINTER.printf("!g!_Actor serialization!! [!rsize:%i!!]:\n" FOS_TAB, buffer.second);
    for (int i = 0; i < buffer.second; i++) {
      FOS_TEST_PRINTER.printf(i % 2 == 0 ? "!m%02X!! " : "!b%02X!! ", buffer.first[i]);
      if ((i + 1) % 10 == 0)
        FOS_TEST_PRINTER.printf("\n" FOS_TAB);
    }
    FOS_TEST_PRINTER.println();
    //delete actor;
    free(clone);
  }

  FOS_RUN_TESTS( //
      // called outside test functions as singletons alter memory
      // across tests
      Scheduler::singleton(); //
      LocalRouter<>::singleton(); //
      FOS_RUN_TEST(test_actor_throughput<LocalRouter<>>); //
      FOS_RUN_TEST(test_actor_by_router<LocalRouter<>>); //
      // FOS_RUN_TEST(test_actor_by_router<MqttRouter<>>);  //
      //  FOS_RUN_TEST(test_actor_by_router<MetaRouter<>>);  //
      FOS_RUN_TEST(test_message_retain<LocalRouter<>>); //
      FOS_RUN_TEST(test_actor_serialization<LocalRouter<>>); //
      );

}

SETUP_AND_LOOP()

#endif
