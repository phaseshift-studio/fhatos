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

#ifndef fhatos_test_program_cpp
#define fhatos_test_program_cpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#define FOS_DEPLOY_EXT
#include <../test/test_fhatos.hpp>
//#include <model/program.hpp>
#include <process/obj_process.hpp>
#include <structure/obj_structure.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  /*void test_actor_throughput() {
    auto counter1 = new std::atomic<int>(0);
    auto counter2 = new std::atomic<int>(0);
    auto actor1 = std::make_shared<Program>(std::make_shared<HeapObj>("",Obj::to_rec()),std::make_shared<ThreadObj>("/app/actor1@127.0.0.1"));
    auto actor2 = std::make_shared<Program>(ID("/app/actor2@127.0.0.1"));
    router()->attach(actor1);
    scheduler()->spawn(actor1);
    router()->attach(actor2);
    scheduler()->spawn(actor2);
    actor1->subscribe(ID("/app/actor1@127.0.0.1/X"), [counter1, counter2, actor1](const ptr<Message> &message) {
      TEST_ASSERT_FALSE(message->retain);
      if (counter1->fetch_add(1) > 198)
        actor1->stop();
      TEST_ASSERT_EQUAL(counter1->load(), counter1->load());
      if (counter1->load() <= 199)
        actor1->publish(ID("/app/actor2@127.0.0.1/X"), jnt(counter1->load()), TRANSIENT_MESSAGE);
    });
    actor2->subscribe(ID("/app/actor2@127.0.0.1/X"), [actor2, counter2](const ptr<Message> &message) {
      TEST_ASSERT_FALSE(message->retain);
      FOS_TEST_ASSERT_EQUAL_FURI(ID(actor2->id()->extend("X")), message->target);
      actor2->publish(ID("/app/actor1@127.0.0.1/X"), jnt(counter2->load()), TRANSIENT_MESSAGE);
      if (counter2->fetch_add(1) > 198)
        actor2->stop();
    });
    scheduler()->barrier("first_barrier", [actor1,actor2] { return actor1->active() && actor2->active(); });
    actor1->publish("/app/actor2@127.0.0.1/X", str("START"), TRANSIENT_MESSAGE);
    Scheduler::singleton()->barrier("no_actors", [] { return Scheduler::singleton()->count("/app/#") == 0; });
    TEST_ASSERT_EQUAL(counter1->load(), counter2->load());
    TEST_ASSERT_EQUAL(200, counter1->load());
    delete counter1;
    delete counter2;
    scheduler()->barrier("last_barrier", [] { return Scheduler::singleton()->count("/app/#") == 0; });
  }

  void test_actor_by_router() {
    auto *counter1 = new std::atomic<int>(0);
    auto *counter2 = new std::atomic<int>(0);
    auto actor1 = std::make_shared<Actor<Thread, Heap>>("/app/actor1@127.0.0.1");
    auto actor2 = std::make_shared<Actor<Thread, Heap>>("/app/actor2@127.0.0.1");
    router()->attach(actor1);
    scheduler()->spawn(actor1);
    router()->attach(actor2);
    scheduler()->spawn(actor2);
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor1@127.0.0.1"), *actor1->id());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/app/actor2@127.0.0.1"), *actor2->id());
    actor1->subscribe("/app/actor1@127.0.0.1/X",
                      [actor1, actor2, counter1, counter2](const ptr<Message> &message) {
                        if (message->payload->is_str()) {
                          TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
                          FOS_TEST_ASSERT_EQUAL_FURI(message->target, actor1->id()->extend("X"));
                          TEST_ASSERT_EQUAL_INT(0, counter1->load());
                          TEST_ASSERT_EQUAL_INT(0, counter2->load());
                          counter1->fetch_add(1);
                          counter2->fetch_add(1);
                          actor1->publish(ID("/app/actor2@127.0.0.1").extend("X"), str("pong"),
                                          TRANSIENT_MESSAGE);
                          TEST_ASSERT_FALSE(message->retain);
                        } else
                          TEST_FAIL_MESSAGE("All message payloads should be strs");
                      });
    actor2->subscribe("/app/actor2@127.0.0.1/X",
                      [actor2, counter1, counter2](const ptr<Message> &message) {
                        if (message->payload->is_str()) {
                          TEST_ASSERT_EQUAL_STRING("pong", message->payload->str_value().c_str());
                          FOS_TEST_ASSERT_EQUAL_FURI(message->target, actor2->id()->extend("X"));
                          TEST_ASSERT_EQUAL_INT(1, counter1->load());
                          TEST_ASSERT_EQUAL_INT(1, counter2->load());
                          TEST_ASSERT_FALSE(message->retain);
                          counter2->fetch_add(1);
                        } else
                          TEST_FAIL_MESSAGE("All message payloads should be strs");
                      });
    TEST_ASSERT_EQUAL(RESPONSE_CODE::REPEAT_SUBSCRIPTION,
                      actor1->subscribe("/app/actor1@127.0.0.1/X", [](const ptr<Message> &message) {
                        TEST_ASSERT_EQUAL_STRING("ping", message->payload->toString().c_str());
                      }));
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
    auto actor1 = std::make_shared<Actor<Thread, Heap>>("/app/actor1@127.0.0.1");
    auto actor2 = std::make_shared<Actor<Thread, Heap>>("/app/actor2@127.0.0.1");
    router()->attach(actor1);
    scheduler()->spawn(actor1);
    router()->attach(actor2);
    scheduler()->spawn(actor2);
    actor1->subscribe(actor1->id()->extend("X"),
                      [actor1, actor2, counter1](const ptr<Message> &message) {
                        if (message->payload->is_str()) {
                          TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
                          FOS_TEST_ASSERT_EQUAL_FURI(message->target, actor1->id()->extend("X"));
                          if (scheduler()->at_barrier("first_barrier"))
                            TEST_ASSERT_LESS_THAN_INT(2, counter1->load());
                          counter1->fetch_add(1);
                          if (scheduler()->at_barrier("second_barrier"))
                            TEST_ASSERT_LESS_THAN_INT(3, counter1->load());
                        }
                      });
    actor2->publish(actor1->id()->extend("X"), str("ping"), RETAIN_MESSAGE);
    scheduler()->barrier("first_barrier", [counter1] { return counter1->load() > 0; });
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(0, counter2->load());
    actor2->subscribe("/app/actor1@127.0.0.1/X", [actor1, counter2](const ptr<Message> &message) {
      if (message->payload->is_str()) {
        TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message->target, actor1->id()->extend("X"));
        counter2->fetch_add(1);
      }
    });
    scheduler()->barrier("second_barrier", [counter2] { return counter2->load() > 0; });
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(1, counter2->load());
    actor1->unsubscribe(p_p(actor1->id()->extend("X")));
    actor1->subscribe("/app/actor1@127.0.0.1/X", [actor1, counter2](const ptr<Message> &message) {
      if (message->payload->is_str()) {
        TEST_ASSERT_EQUAL_STRING("ping", message->payload->str_value().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(message->target, actor1->id()->extend("X"));
        counter2->fetch_add(1);
      }
    });
    TEST_ASSERT_EQUAL_INT(1, counter1->load());
    TEST_ASSERT_EQUAL_INT(2, counter2->load());
    actor1->stop();
    actor2->stop();
    scheduler()->barrier("last_barrier", [] { return scheduler()->count("/app/#") == 0; });
    delete counter1;
    delete counter2;
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_actor_throughput); //
    FOS_RUN_TEST(test_actor_by_router); //
    FOS_RUN_TEST(test_message_retain); //
  );*/
} // namespace fhatos

/*SETUP_AND_LOOP()*/

#endif
