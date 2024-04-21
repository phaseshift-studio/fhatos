#ifndef fhatos_kernel__test_actor_hpp
#define fhatos_kernel__test_actor_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/broker/broker.hpp>
#include <kernel/process/actor/broker/local_broker/local_broker.hpp>
#include <kernel/process/task/esp32/scheduler.hpp>
#include <kernel/process/task/esp32/thread.hpp>
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_actor_communication() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 =
      new Actor<Thread, StringMessage, LocalBroker<StringMessage>, String>(
          "actor1@127.0.0.1");
  auto *actor2 =
      new Actor<Thread, StringMessage, LocalBroker<StringMessage>, String>(
          "actor2@127.0.0.1");

  TEST_ASSERT_EQUAL_FURI(fURI("actor1@127.0.0.1"), actor1->id());
  TEST_ASSERT_EQUAL_FURI(fURI("actor2@127.0.0.1"), actor2->id());
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(
          actor1->id(), [actor1, actor2, counter](StringMessage message) {
            TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
            TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
            TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
            TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                              actor1->publish(message.source, "pong", false));
            counter->first++;
            counter->second++;
          }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe(
          "actor2@127.0.0.1", [actor1, actor2, counter](StringMessage message) {
            TEST_ASSERT_EQUAL_STRING("pong", message.payload.c_str());
            TEST_ASSERT_EQUAL_FURI(message.source, actor1->id());
            TEST_ASSERT_EQUAL_FURI(message.target, actor2->id());
            counter->second++;
          }));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::REPEAT_SUBSCRIPTION,
      actor1->subscribe("actor1@127.0.0.1", [](StringMessage message) {
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
}

void test_message_retain() {
  Pair<int, int> *counter = new Pair<int, int>{0, 0};
  auto *actor1 =
      new Actor<Thread, StringMessage, LocalBroker<StringMessage>, String>(
          "actor1@127.0.0.1");
  auto *actor2 =
      new Actor<Thread, StringMessage, LocalBroker<StringMessage>, String>(
          "actor2@127.0.0.1");

  TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                    actor1->subscribe(actor1->id(), [actor1, actor2, counter](
                                                        StringMessage message) {
                      TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
                      TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
                      TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
                      counter->first++;
                    }));
  actor2->publish(*actor1, "ping", true);
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(0, counter->second);
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor2->subscribe(
          "actor1@127.0.0.1", [actor1, actor2, counter](StringMessage message) {
            TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
            TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
            TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
            counter->second++;
          }));
  actor1->loop();
  actor2->loop();
  TEST_ASSERT_EQUAL_INT(1, counter->first);
  TEST_ASSERT_EQUAL_INT(1, counter->second);
  TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe(actor1->id()));
  TEST_ASSERT_EQUAL(
      RESPONSE_CODE::OK,
      actor1->subscribe(
          "actor1@127.0.0.1", [actor1, actor2, counter](StringMessage message) {
            TEST_ASSERT_EQUAL_STRING("ping", message.payload.c_str());
            TEST_ASSERT_EQUAL_FURI(message.source, actor2->id());
            TEST_ASSERT_EQUAL_FURI(message.target, actor1->id());
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
}

RUN_TESTS(                              //
    RUN_TEST(test_actor_communication); //
    RUN_TEST(test_message_retain);      //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
