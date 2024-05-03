#ifndef fhatos_kernel__test_actor_hpp
#define fhatos_kernel__test_actor_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/router/local_router.hpp>
#include <kernel/process/router/meta_router.hpp>
#include <kernel/process/router/router.hpp>
#include <kernel/process/esp32/scheduler.hpp>
#include <kernel/process/esp32/thread.hpp>
#include <kernel/furi.hpp>
#include <unity.h>

namespace fhatos::kernel {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

template <typename ROUTER> void test_actor_by_router() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 = new Actor<Thread, String, ROUTER>("actor1@127.0.0.1");
  auto *actor2 = new Actor<Thread, String, ROUTER>("actor2@127.0.0.1");

  FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor1@127.0.0.1"), actor1->id());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor2@127.0.0.1"), actor2->id());
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(actor1->id(), [actor1, actor2,
                                       counter](const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
        TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                          actor1->publish(message.source, "pong", false));
        counter->first++;
        counter->second++;
      }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe("actor2@127.0.0.1", [actor1, actor2, counter](
                                                const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("pong", message.payload.c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor1->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor2->id());
        counter->second++;
      }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::REPEAT_SUBSCRIPTION,
      actor1->subscribe("actor1@127.0.0.1", [](const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
      }));

  actor2->publish(*actor1, "ping", false);
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

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

template <typename ROUTER> void test_message_retain() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 = new Actor<Thread, String, ROUTER>("actor1@127.0.0.1");
  auto *actor2 = new Actor<Thread, String, ROUTER>("actor2@127.0.0.1");

  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(actor1->id(), [actor1, actor2,
                                       counter](const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
        FOS_TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
        counter->first++;
      }));
  actor2->publish(*actor1, "ping", RETAIN_MESSAGE);
  actor1->loop();
  actor2->loop();
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(0, counter->second);
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe("actor1@127.0.0.1", [actor1, actor2, counter](
                                                const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
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
      actor1->subscribe("actor1@127.0.0.1", [actor1, actor2, counter](
                                                const StringMessage &message) {
        TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
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

FOS_RUN_TESTS( //
               // called outside test functions as singletons alter memory
               // across tests
    LocalRouter<>::singleton(); //
    // MqttRouter<StringMessage>::singleton();  //
    // MetaRouter<StringMessage>::singleton();  //
    FOS_RUN_TEST(test_actor_by_router<LocalRouter<>>); //
    // FOS_RUN_TEST(test_actor_by_router<MqttRouter<StringMessage>>);  //
    // FOS_RUN_TEST(test_actor_by_router<MetaRouter<StringMessage>>);  //
    FOS_RUN_TEST(test_message_retain<LocalRouter<>>); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
