/*******************************************************************************
  FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_test_actor_hpp
#define fhatos_test_actor_hpp

#define FOS_TEST_ON_BOOT

#include <test_fhatos.hpp>


namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  /*void test_actor_throughput() {
    auto counter1 = new std::atomic<int>(0);
    auto counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread, Empty>(ID("/app/actor1@127.0.0.1"), [counter1](Actor<Thread> *self) {
      self->subscribe(ID("/app/actor1@127.0.0.1"), [counter1, self](const ptr<Message> &) {
        self->publish(ID("/app/actor2@127.0.0.1"), share(Int(counter1->load())), TRANSIENT_MESSAGE);
        if (counter1->fetch_add(1) > 198)
          self->stop();
        // TEST_ASSERT_EQUAL(counter1->first, counter1->second);
      });
    });
    auto *actor2 = new Actor<Thread, Empty>(ID("/app/actor2@127.0.0.1"), [counter2](Actor<Thread> *self) {
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
    TEST_ASSERT_EQUAL(counter1->load(), counter2->load());
    TEST_ASSERT_EQUAL(200, counter1->load());
    delete counter1;
    delete counter2;
    delete actor1;
    delete actor2;
  }*/

  void test_actor_by_router() {
    auto *counter1 = new std::atomic<int>(0);
    auto *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread, KeyValue>("/app/actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread, KeyValue>("/app/actor2@127.0.0.1");
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
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    rc = actor2->subscribe("/app/actor2@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("pong", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor1->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor2->id());
      counter2->fetch_add(1);
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    TEST_ASSERT_EQUAL(RESPONSE_CODE::REPEAT_SUBSCRIPTION,
                      actor1->subscribe("/app/actor1@127.0.0.1", [](const ptr<Message> &message) {
                        TEST_ASSERT_EQUAL_STRING("ping", message->payload->toString().c_str());
                      }));

    actor2->publish(*actor1->id(), share(Str("ping")), TRANSIENT_MESSAGE);
    actor1->loop();
    actor2->loop();
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    // Options::singleton()->router<Router>()->clear();
    delete counter1;
    delete counter2;
    Scheduler::singleton()->barrier("here", []() { return Scheduler::singleton()->count("/app/#") == 0; });
  }

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_message_retain() {
    auto *counter1 = new std::atomic<int>(0);
    auto *counter2 = new std::atomic<int>(0);
    auto *actor1 = new Actor<Thread, KeyValue>("/app/actor1@127.0.0.1");
    auto *actor2 = new Actor<Thread, KeyValue>("/app/actor2@127.0.0.1");
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
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
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
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    actor1->loop();
    actor2->loop();
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(1, counter2->load());
    //  TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe(actor1->id()));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe());
    rc = actor1->subscribe("/app/actor1@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
      FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
      counter2->fetch_add(1);
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
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
    // Options::singleton()->router<Router>()->clear();
  }

  FOS_RUN_TESTS( //
  // FOS_RUN_TEST(test_actor_throughput); //
          FOS_RUN_TEST(test_actor_by_router); //
          FOS_RUN_TEST(test_message_retain); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
