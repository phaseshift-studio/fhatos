#ifndef fhatos_test_actor_hpp
#define fhatos_test_actor_hpp

#include <test_fhatos.hpp>
//
#include <furi.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <process/router/local_router.hpp>
#include <process/router/meta_router.hpp>
#include <process/router/router.hpp>

namespace fhatos::kernel {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

template <typename ROUTER> void test_actor_throughput() {
  auto *counter = new Pair<int, int>{0, 0};
  Actor<Thread, ROUTER> *actor1 = new Actor<Thread, ROUTER>(
      ID("actor1@127.0.0.1"), [counter](Actor<Thread, ROUTER> *self) {
        self->subscribe(ID("actor1@"), [counter, self](const Message &message) {
          self->publish(ID("actor2@"), counter->first, TRANSIENT_MESSAGE);
          if (counter->first++ > 198)
            self->stop();
          // TEST_ASSERT_EQUAL(counter1->first, counter1->second);
        });
      });
  Actor<Thread, ROUTER> *actor2 = new Actor<Thread, ROUTER>(
      ID("actor2@127.0.0.1"), [counter](Actor<Thread, ROUTER> *self) {
        self->subscribe(ID("actor2@"), [self, counter](const Message &message) {
          FOS_TEST_ASSERT_EQUAL_FURI(ID("actor1@127.0.0.1"), message.source);
          FOS_TEST_ASSERT_EQUAL_FURI(self->id(), message.target);
          self->publish(ID("actor1@"), counter->second, TRANSIENT_MESSAGE);
          if (counter->second++ > 198)
            self->stop();
        });
      });

  Scheduler<ROUTER>::singleton()->spawn(actor1);
  Scheduler<ROUTER>::singleton()->spawn(actor2);
  actor1->publish("actor2@", "START!", TRANSIENT_MESSAGE);
  while (Scheduler<ROUTER>::singleton()->count("actor1@127.0.0.1") ||
         Scheduler<ROUTER>::singleton()->count("actor2@127.0.0.1")) {
  }
  ROUTER::singleton()->clear();
  TEST_ASSERT_EQUAL(counter->first + 1, counter->second);
  TEST_ASSERT_EQUAL(200, counter->first);
  delete counter;
  // delete Scheduler<ROUTER>::singleton();
  // delete ROUTER::singleton();
}

template <typename ROUTER> void test_actor_by_router() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 = new Actor<Thread, ROUTER>("actor1@127.0.0.1");
  auto *actor2 = new Actor<Thread, ROUTER>("actor2@127.0.0.1");
  actor1->setup();
  actor2->setup();

  FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor1@127.0.0.1"), actor1->id());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor2@127.0.0.1"), actor2->id());
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(
          actor1->id(), [actor1, actor2, counter](const Message &message) {
            TEST_ASSERT_EQUAL_STRING("ping", (char *)message.payload.data);
            FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
            FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
            TEST_ASSERT_EQUAL(
                RESPONSE_CODE::OK,
                actor1->publish(message.source, "pong", TRANSIENT_MESSAGE));
            counter->first++;
            counter->second++;
          }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe("actor2@127.0.0.1", [actor1, actor2,
                                             counter](const Message &message) {
        TEST_ASSERT_EQUAL_STRING("pong", (char *)message.payload.data);
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor1->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor2->id());
        counter->second++;
      }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::REPEAT_SUBSCRIPTION,
      actor1->subscribe("actor1@127.0.0.1", [](const Message &message) {
        TEST_ASSERT_EQUAL_STRING("ping", (char *)message.payload.data);
      }));

  actor2->publish(actor1->id(), "ping", TRANSIENT_MESSAGE);
  actor1->loop();
  actor2->loop();
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(2, counter->second);
  actor1->stop();
  actor2->stop();
  actor1->serialize();
  delete actor1;
  delete actor2;
  delete counter;
  ROUTER::singleton()->clear();
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

template <typename ROUTER> void test_message_retain() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 = new Actor<Thread, ROUTER>("actor1@127.0.0.1");
  auto *actor2 = new Actor<Thread, ROUTER>("actor2@127.0.0.1");
  actor1->setup();
  actor2->setup();
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(actor1->id(), [actor1, actor2,
                                       counter](const Message &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
        counter->first++;
      }));
  actor2->publish(actor1->id(), "ping", RETAIN_MESSAGE);
  actor1->loop();
  actor2->loop();
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(0, counter->second);
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe("actor1@127.0.0.1", [actor1, actor2,
                                             counter](const Message &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
        counter->second++;
      }));
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(1, counter->second);
  TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe(actor1->id()));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe("actor1@127.0.0.1", [actor1, actor2,
                                             counter](const Message &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
        counter->second++;
      }));
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(1, counter->second);
  actor1->loop();
  actor2->loop();

  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(2, counter->second);
  actor1->stop();
  actor2->stop();
  delete actor1;
  delete actor2;
  delete counter;
  ROUTER::singleton()->clear();
}

template <typename ROUTER> void test_actor_serialization() {
  Actor<Thread, ROUTER> *actor = new Actor<Thread, ROUTER>("abc");
  const Pair<byte *, uint> buffer = actor->serialize();
  Actor<Thread, ROUTER> *clone =
      Actor<Thread, ROUTER>::deserialize(buffer.first);
  FOS_TEST_ASSERT_EQUAL_FURI(actor->id(), clone->id());
  FOS_TEST_PRINTER.printf("!g!_Actor serialization!! [!rsize:%i!!]:\n" FOS_TAB, buffer.second);
  for (int i = 0; i < buffer.second; i++) {
    FOS_TEST_PRINTER.printf(i % 2 == 0 ? "!m%02X!! " : "!b%02X!! ", buffer.first[i]);
    if ((i+1) % 10 == 0)
      FOS_TEST_PRINTER.printf("\n" FOS_TAB);
  }
  FOS_TEST_PRINTER.println();
  delete buffer.first;
}

FOS_RUN_TESTS( //
               // called outside test functions as singletons alter memory
               // across tests
    auto *s = Scheduler<LocalRouter<>>::singleton(); //
    auto *l = LocalRouter<>::
        singleton(); //
                     // Scheduler<MqttRouter<>>::singleton()->spawn(MqttRouter<>::singleton());
                     // //
                     //  MetaRouter<>::singleton();  //
    FOS_RUN_TEST(test_actor_throughput<LocalRouter<>>); //
    FOS_RUN_TEST(test_actor_by_router<LocalRouter<>>);  //
    // FOS_RUN_TEST(test_actor_by_router<MqttRouter<>>);  //
    //  FOS_RUN_TEST(test_actor_by_router<MetaRouter<>>);  //
    FOS_RUN_TEST(test_message_retain<LocalRouter<>>);      //
    FOS_RUN_TEST(test_actor_serialization<LocalRouter<>>); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
