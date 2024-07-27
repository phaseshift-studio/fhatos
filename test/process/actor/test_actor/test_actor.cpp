#ifndef fhatos_test_actor_hpp
#define fhatos_test_actor_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>


namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  template<typename ROUTER>
  void test_actor_throughput() {
    auto counter1 = new std::atomic<int>(0);
    auto counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread>(ID("/app/actor1@127.0.0.1"), [counter1](Actor<Thread> *self) {
      self->subscribe(ID("/app/actor1@127.0.0.1"), [counter1, self](const ptr<Message> &message) {
        self->publish(ID("/app/actor2@127.0.0.1"), share(Int(counter1->load())), TRANSIENT_MESSAGE);
        if (counter1->fetch_add(1) > 198)
          self->stop();
        // TEST_ASSERT_EQUAL(counter1->first, counter1->second);
      });
    });
    auto *actor2 = new Actor<Thread>(ID("/app/actor2@127.0.0.1"), [counter2](Actor<Thread> *self) {
      self->subscribe(ID("/app/actor2@127.0.0.1"), [self, counter2](const ptr<Message> &message) {
        FOS_TEST_ASSERT_EQUAL_FURI(ID("/app/actor1@127.0.0.1"), message->source);
        FOS_TEST_ASSERT_EQUAL_FURI(*self->id(), message->target);
        self->publish(ID("/app/actor1@127.0.0.1"), share(Int(counter2->load())), TRANSIENT_MESSAGE);
        if (counter2->fetch_add(1) > 198)
          self->stop();
      });
    });
    Scheduler::singleton()->spawn(actor1);
    Scheduler::singleton()->spawn(actor2);
    actor1->publish("/app/actor2@127.0.0.1", share(Str("START")), TRANSIENT_MESSAGE);
    Scheduler::singleton()->barrier("no_actors", [] { return Scheduler::singleton()->count("/app/#") == 0; });
    ROUTER::singleton()->clear();
    TEST_ASSERT_EQUAL(counter1->load(), counter2->load());
    TEST_ASSERT_EQUAL(200, counter1->load());
    // delete counter1;
    // delete counter2;
  }

  void test_actor_by_router() {
    std::atomic<int> *counter1 = new std::atomic<int>(0);
    std::atomic<int> *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread>("/app/actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread>("/app/actor2@127.0.0.1");
    actor1->setup();
    actor2->setup();
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor1@127.0.0.1"), *actor1->id());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor2@127.0.0.1"), *actor2->id());
    RESPONSE_CODE rc = actor1->subscribe("", [actor1, actor2, counter1, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
      TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                        actor1->publish(message->source, share<Str>(Str("pong")), TRANSIENT_MESSAGE));
      counter1->fetch_add(1);
      counter2->fetch_add(1);
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
    rc = actor2->subscribe("/app/actor2@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("pong", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor1->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor2->id());
      counter2->fetch_add(1);
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(OK, rc);
   /* TEST_ASSERT_EQUAL(RESPONSE_CODE::REPEAT_SUBSCRIPTION,
                      actor1->subscribe("/app/actor1@127.0.0.1", [](const ptr<Message> &message) {
                        TEST_ASSERT_EQUAL_STRING("ping", message->payload->toString().c_str());
                      }));*/

    actor2->publish(*actor1->id(), share(Str("ping")), TRANSIENT_MESSAGE);
    actor1->loop();
    actor2->loop();
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    GLOBAL_OPTIONS->router<Router>()->clear();
    // delete counter1;
    // delete counter2;
    Scheduler::singleton()->barrier("here", []() { return Scheduler::singleton()->count("/app/#") == 0; });
  }

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_message_retain() {
    auto *counter1 = new std::atomic<int>(0);
    auto *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread>("/app/actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread>("/app/actor2@127.0.0.1");
    actor1->setup();
    actor2->setup();

    RESPONSE_CODE rc = actor1->subscribe(*actor1->id(), [actor1, actor2, counter1](const ptr<Message> &message) {
      TEST_ASSERT_TRUE(Str("ping") == *(Str *) message->payload.get());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
      TEST_ASSERT_EQUAL_INT(0, counter1->load());
      counter1->fetch_add(1);
      TEST_ASSERT_EQUAL_INT(1, counter1->load());
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", RESPONSE_CODE_STR(rc));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, rc);
    actor2->publish(*actor1->id(), share<Str>(Str("ping")), RETAIN_MESSAGE);
    actor1->loop();
    actor2->loop();
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(0, counter2->load());
    rc = actor2->subscribe("/app/actor1@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
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
    rc = actor1->subscribe("/app/actor1@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
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
    // delete counter1;
    // delete counter2;
    GLOBAL_OPTIONS->router<Router>()->clear();
  }

  void test_actor_serialization() {
    auto *actor = new Actor<Thread>("/abc");
    const Pair<fbyte *, uint> buffer = actor->serialize();
    Actor<Thread> *clone = Actor<Thread>::deserialize(buffer.first);
    FOS_TEST_ASSERT_EQUAL_FURI(*actor->id(), *clone->id());
    GLOBAL_OPTIONS->printer<>()->printf("!g!_Actor serialization!! [!rsize:%i!!]:\n" FOS_TAB, buffer.second);
    for (int i = 0; i < buffer.second; i++) {
      GLOBAL_OPTIONS->printer<>()->printf(i % 2 == 0 ? "!m%02X!! " : "!b%02X!! ", buffer.first[i]);
      if ((i + 1) % 10 == 0)
        GLOBAL_OPTIONS->printer<>()->printf("\n" FOS_TAB);
    }
    GLOBAL_OPTIONS->printer<>()->println();
    // delete actor;
    // free(clone);
    Scheduler::singleton()->barrier("done", []() { return Scheduler::singleton()->count("/abc") == 0; });
  }

  FOS_RUN_TESTS( //
      for (Router *router //
           : List<Router *>{/*FOS_TEST_ROUTERS*/LocalRouter::singleton()}) {
        //
        GLOBAL_OPTIONS->ROUTING = router; //
        router->clear();
        LOG(INFO, "!r!_Testing with %s!!\n", router->toString().c_str()); //
        FOS_RUN_TEST(test_actor_by_router); //
        FOS_RUN_TEST(test_message_retain); //
        FOS_RUN_TEST(test_actor_serialization); //
      } //
  );

} // namespace fhatos

SETUP_AND_LOOP()

#endif
