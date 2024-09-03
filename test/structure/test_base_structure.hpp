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

#ifndef fhatos_test_base_structure_hpp
#define fhatos_test_base_structure_hpp

#undef FOS_TEST_ON_BOOT
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#include <test_fhatos.hpp>

#include "../../build/_deps/unity-src/src/unity.h"

#define FOS_TEST_PATTERN STR(/a/+)

namespace fhatos {
  inline Structure_p current_structure;
  inline bool auto_loop;

  void begin_test_structure(const Structure_p &test_structure, const bool autoloop = true) {
    current_structure = test_structure;
    auto_loop = autoloop;;
    TEST_ASSERT_EQUAL_STRING(FOS_TEST_PATTERN, current_structure->pattern()->toString().c_str());
    LOG(INFO, "!yStarting!! test_base_structure !yon !g%s!y using !g%s!y pattern\n",
        current_structure->pattern()->toString().c_str(),
        FOS_TEST_PATTERN);
    router()->attach(current_structure);
    current_structure->setup();
  }

  void end_test_structure() {
    current_structure->stop();
    router()->detach(current_structure->pattern());
  }

  ////////////////////////////////

  void test_write() {
    auto *ping_HIT = new atomic_int(0);
    auto *ping_MISS = new atomic_int(0);
    const Subscription_p subscription_HIT = share(Subscription{
      .source = "tester_HIT", .pattern = "/a/+", .onRecv = [ping_HIT](const Message_p &message) {
        LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(ID("/a/b"), message->target);
        TEST_ASSERT_TRUE(message->payload->is_rec());
        FL_INT_TYPE payload_int = message->payload->rec_value()->at(str("hello_fhatty"))->int_value();
        TEST_ASSERT_EQUAL_INT(payload_int, ping_HIT->load());
        TEST_ASSERT_TRUE(message->retain);
        ping_HIT->store(ping_HIT->load() + 1);
        FOS_TEST_ASSERT_EQUAL_FURI(ID((string("piggy") + to_string(payload_int)).c_str()), message->source);
      }
    });
    const Subscription_p subscription_MISS = share(Subscription{
      .source = "tester_MISS", .pattern = "/a/c", .onRecv = [ping_MISS](const Message_p &message) {
        ping_MISS->store(ping_MISS->load() + 1);
        LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
        TEST_FAIL_MESSAGE(
          (string("Subscription /a/c does not match payload target:") + message->target.toString()).c_str());
      }
    });
    router()->route_subscription(subscription_HIT);
    router()->route_subscription(subscription_MISS);
    if (auto_loop)
      current_structure->loop();
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/b/c"), jnt(10), id_p("fhatty")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("a/b/c"), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/b/c"), str("hello_fhatty"), id_p("aus")));
    // TODO: FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/"), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a"), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/"), str("hello_fhatty"), id_p("aus")));
    for (int i = 0; i < 10; i++) {
      if (auto_loop)
        current_structure->loop();
      TEST_ASSERT_EQUAL_INT(i, ping_HIT->load());
      router()->write(id_p("/a/b"), rec({{"hello_fhatty", i}}),
                      id_p((string("piggy") + to_string(i)).c_str()));
    }
    router()->route_unsubscribe(id_p("tester_HIT"), p_p("/a/b"));
    if (auto_loop)
      current_structure->loop();
    // should not cause an exception due to str != rec as no subscription exists
    router()->write(id_p("/a/b"), str("hello_fhatty"), id_p("not_there"));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(10, ping_HIT->load());
    TEST_ASSERT_EQUAL_INT(0, ping_MISS->load());
    router()->route_unsubscribe(id_p("tester_MISS"), p_p("/a/c"));
    if (auto_loop)
      current_structure->loop();
    router()->write(id_p("/a/c"), str("hello_fhatty"), id_p("not_there"));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(10, ping_HIT->load());
    TEST_ASSERT_EQUAL_INT(0, ping_MISS->load());
    delete ping_HIT;
    delete ping_MISS;
  }

  void test_read() {
    router()->write(id_p("/a/x"), str("good"));
    router()->write(id_p("/a/y"), jnt(6));
    router()->write(id_p("/a/z"), uri("/a/x"));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_STRING("good",
                             router()->read(id_p("/a/x"))->str_value().c_str());
    TEST_ASSERT_EQUAL_INT(Obj::to_int(6)->int_value(), router()->read(id_p("/a/y"))->int_value());
    FOS_TEST_ASSERT_EQUAL_FURI(uri("/a/x")->uri_value(),
                               router()->read(id_p("/a/z"))->uri_value());
    TEST_ASSERT_EQUAL_STRING("good",
                             router()->read(share(router()->read(id_p("/a/z"))->uri_value()))->str_value().
                             c_str());
  }

  void test_subscribe() {
    //Options::singleton()->log_level(TRACE);
    auto *pings = new atomic_int(0);
    Consumer<Message_p> on_recv_ = [pings](const Message_p &message) {
      FOS_TEST_ASSERT_EQUAL_FURI(Pattern("/a/test"), message->target);
      if (message->payload->is_bool()) {
        TEST_ASSERT_TRUE(message->payload->bool_value());
        pings->store(pings->load() + 1);
        TEST_ASSERT_EQUAL(1, pings->load());
      }
    };
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(0, pings->load());
    FOS_TEST_EXCEPTION_CXX(router()->route_subscription(share(Subscription{
      .source = "a/test/case", .pattern = "/a/test/bad", .qos = QoS::_1, .onRecv = on_recv_
      })));
    RESPONSE_CODE rc_ = router()->route_subscription(share(Subscription{
      .source = "a/test/case", .pattern = "/a/test", .qos = QoS::_1, .onRecv = on_recv_
    }));
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, rc_);
    router()->route_message(share(Message{
      .source = "b/test/case", .target = ID("/a/test"), .payload = Obj::to_bool(true), .retain = TRANSIENT_MESSAGE
    }));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(1, pings->load());
    router()->route_unsubscribe(id_p("a/test/case"),p_p("/a/test"));
    if(auto_loop)
      current_structure->loop();
    router()->route_message(share(Message{
      .source = "b/test/case", .target = ID("/a/test"), .payload = Obj::to_bool(true), .retain = TRANSIENT_MESSAGE
    }));
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    TEST_ASSERT_EQUAL_INT(1, pings->load()); // shouldn't change as subscribe has unsubscribed by now
    delete pings;
  }
} // namespace fhatos

#endif
