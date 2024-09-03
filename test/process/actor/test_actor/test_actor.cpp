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

#undef FOS_TEST_ON_BOOT
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#include <test_fhatos.hpp>
#include <process/actor/actor.hpp>

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
    auto actor1 = ptr<Actor<Thread, KeyValue>>(new Actor<Thread, KeyValue>("/app/actor1@127.0.0.1"));
    auto actor2 = ptr<Actor<Thread, KeyValue>>(new Actor<Thread, KeyValue>("/app/actor2@127.0.0.1"));
    Model::deploy(ptr<Actor<Thread, KeyValue>>(actor1));
    Model::deploy(ptr<Actor<Thread, KeyValue>>(actor2));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor1@127.0.0.1"), *actor1->id());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor2@127.0.0.1"), *actor2->id());
    RESPONSE_CODE rc = actor1->subscribe("/app/actor1@127.0.0.1/X",
                                         [actor1, actor2, counter1, counter2](const ptr<Message> &message) {
                                           if (message->payload->is_str()) {
                                             TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
                                             FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
                                             //FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
                                             TEST_ASSERT_EQUAL_INT(0, counter1->load());
                                             TEST_ASSERT_EQUAL_INT(0, counter2->load());
                                             counter1->fetch_add(1);
                                             counter2->fetch_add(1);
                                             actor1->publish(message->source.extend("X"), str("pong"),
                                                             TRANSIENT_MESSAGE);
                                             TEST_ASSERT_FALSE(message->retain);
                                           } else if (message->payload->is_noobj()) {
                                             //actor1->stop();
                                           }
                                         });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    rc = actor2->subscribe("/app/actor2@127.0.0.1/X",
                           [actor1, actor2, counter1, counter2](const ptr<Message> &message) {
                             if (message->payload->is_str()) {
                               TEST_ASSERT_EQUAL_STRING("pong", message->payload->str_value().c_str());
                               FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor1->id());
                               //FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor2->id());
                               TEST_ASSERT_EQUAL_INT(1, counter1->load());
                               TEST_ASSERT_EQUAL_INT(1, counter2->load());
                               TEST_ASSERT_FALSE(message->retain);
                               counter2->fetch_add(1);
                             } else if (message->payload->is_noobj()) {
                               //actor2->stop();
                             }
                           });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    /*TEST_ASSERT_EQUAL(RESPONSE_CODE::REPEAT_SUBSCRIPTION,
                      actor1->subscribe("/app/actor1@127.0.0.1/X", [](const ptr<Message> &message) {
                        TEST_ASSERT_EQUAL_STRING("ping", message->payload->toString().c_str());
                      }));*/
    actor2->publish(actor1->id()->extend("X"), str("ping"), TRANSIENT_MESSAGE);
    scheduler()->barrier("first_barrier", [counter1, counter2] {
      return counter2->load() > 1 && counter1->load() > 0;
    });
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    delete counter1;
    delete counter2;
    actor1->stop();
    actor2->stop();
    scheduler()->barrier("last_barrier", [] { return Scheduler::singleton()->count("/app/#") == 0; });
  }

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_message_retain() {
    auto *counter1 = new std::atomic<int>(0);
    auto *counter2 = new std::atomic<int>(0);
    auto actor1 = ptr<Actor<Thread, KeyValue>>(new Actor<Thread, KeyValue>("/app/actor1@127.0.0.1"));
    auto actor2 = ptr<Actor<Thread, KeyValue>>(new Actor<Thread, KeyValue>("/app/actor2@127.0.0.1"));
    Model::deploy(actor1);
    Model::deploy(actor2);
    RESPONSE_CODE rc = actor1->subscribe(*actor1->id(),
                                         [actor1, actor2, counter1](const ptr<Message> &message) {
                                           if (message->payload->is_str()) {
                                             TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
                                             FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
                                             FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
                                             if (scheduler()->at_barrier("first_barrier"))
                                               TEST_ASSERT_LESS_THAN_INT(2, counter1->load());
                                             counter1->fetch_add(1);
                                             if (scheduler()->at_barrier("second_barrier"))
                                               TEST_ASSERT_LESS_THAN_INT(3, counter1->load());
                                           }
                                         });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, rc);
    actor2->publish(*actor1->id(), str("ping"), RETAIN_MESSAGE);
    scheduler()->barrier("first_barrier", [counter1] { return counter1->load() > 0; });
    //TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(0, counter2->load());
    rc = actor2->subscribe("/app/actor1@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      if (message->payload->is_str()) {
        TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
        // FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
        //FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
        counter2->fetch_add(1);
      }
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    scheduler()->barrier("second_barrier", [counter2] { return counter2->load() > 0; });
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(1, counter2->load());
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, actor1->unsubscribe(p_p(*actor1->id())));
    rc = actor1->subscribe("/app/actor1@127.0.0.1", [actor1, actor2, counter2](const ptr<Message> &message) {
      if (message->payload->is_str()) {
        TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
        //FOS_TEST_ASSERT_EQUAL_FURI(message->source, *actor2->id());
        //FOS_TEST_ASSERT_EQUAL_FURI(message->target, *actor1->id());
        counter2->fetch_add(1);
      }
    });
    FOS_TEST_MESSAGE("!RResponse code!!: %s\n", ResponseCodes.toChars(rc).c_str());
    TEST_ASSERT_EQUAL(OK, rc);
    //TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    actor1->stop();
    actor2->stop();
    scheduler()->barrier("last_barrier", [] { return scheduler()->count("/app/#") == 0; });
    delete counter1;
    delete counter2;
  }

  FOS_RUN_TESTS( //
    // FOS_RUN_TEST(test_actor_throughput); //
    FOS_RUN_TEST(test_actor_by_router); //
    FOS_RUN_TEST(test_message_retain); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
