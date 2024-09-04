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

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#include <test_fhatos.hpp>

#include "../../build/_deps/unity-src/src/unity.h"

#define FOS_TEST_PATTERN_PREFIX STR(/a/)

namespace fhatos {
  inline Structure_p current_structure;
  inline bool auto_loop;

  inline fURI_p make_test_pattern(const char *suffix) {
    return furi_p(string(FOS_TEST_PATTERN_PREFIX) + suffix);
  }

  inline void begin_test_structure(const Structure_p &test_structure, const bool autoloop = true) {
    current_structure = test_structure;
    auto_loop = autoloop;;
    TEST_ASSERT_EQUAL_STRING(make_test_pattern("+")->toString().c_str(),
                             current_structure->pattern()->toString().c_str());
    LOG(INFO, "!yStarting!! test_base_structure !yon !g%s!y using !g%s!y pattern\n",
        current_structure->pattern()->toString().c_str(),
        make_test_pattern("+")->toString().c_str());
    router()->attach(current_structure);
    current_structure->setup();
  }

  inline void end_test_structure() {
    current_structure->stop();
    router()->detach(current_structure->pattern());
  }

  ////////////////////////////////

  inline void test_write() {
    auto *ping_HIT = new atomic_int(0);
    auto *ping_MISS = new atomic_int(0);
    const Subscription_p subscription_HIT = share(Subscription{
      .source = "tester_HIT", .pattern = *make_test_pattern("+"), .onRecv = [ping_HIT](const Message_p &message) {
        LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(*make_test_pattern("b"), message->target);
        TEST_ASSERT_TRUE(message->payload->is_rec());
        FL_INT_TYPE payload_int = message->payload->rec_value()->at(str("hello_fhatty"))->int_value();
        TEST_ASSERT_EQUAL_INT(payload_int, ping_HIT->load());
        // TEST_ASSERT_TRUE(message->retain);
        ping_HIT->store(ping_HIT->load() + 1);
        FOS_TEST_ASSERT_EQUAL_FURI(ID((string("piggy") + to_string(payload_int)).c_str()), message->source);
      }
    });
    const Subscription_p subscription_MISS = share(Subscription{
      .source = "tester_MISS", .pattern = *make_test_pattern("c"), .onRecv = [ping_MISS](const Message_p &message) {
        ping_MISS->store(ping_MISS->load() + 1);
        LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
        TEST_FAIL_MESSAGE(
          (string("Subscription ") + make_test_pattern("c")->toString() + " does not match payload target:" + message->
            target.toString(
            )).c_str());
      }
    });
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->route_subscription(subscription_HIT));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->route_subscription(subscription_MISS));
    if (auto_loop)
      current_structure->loop();
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/b/c"), jnt(10), id_p("fhatty")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p(*make_test_pattern("b/c")), str("hello_fhatty"), id_p("aus")));
    //FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/b/c"), str("hello_fhatty"), id_p("aus")));
    // TODO: FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/"), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p(*make_test_pattern("a/a/b/c")), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/"), str("hello_fhatty"), id_p("aus")));
    for (int i = 0; i < 10; i++) {
      TEST_ASSERT_EQUAL_INT(i, ping_HIT->load());
      TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->write(id_p(*make_test_pattern("b")), rec({{"hello_fhatty", i}}),
                          id_p((string("piggy") + to_string(i)).c_str())));
      if (auto_loop)
        current_structure->loop();
      scheduler()->barrier("waiting for messages in test_write #1", [ping_HIT,i]() {
        if (auto_loop)
          current_structure->loop();
        return ping_HIT->load() > i;
      });
    }
    router()->route_unsubscribe(id_p("tester_HIT"), p_p(*make_test_pattern("+")));
    if (auto_loop)
      current_structure->loop();
    // should not cause an exception due to str != rec as no subscription exists
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                      router()->write(id_p(*make_test_pattern("b")), str("hello_fhatty"), id_p("not_there")));
    scheduler()->barrier("waiting for messages in test_write #2", [ping_HIT]() {
      if (auto_loop)
        current_structure->loop();
      return ping_HIT->load() > 9;
    });
    TEST_ASSERT_EQUAL_INT(10, ping_HIT->load());
    TEST_ASSERT_EQUAL_INT(0, ping_MISS->load());
    router()->route_unsubscribe(id_p("tester_MISS"), p_p(*make_test_pattern("c")));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,
                      router()->write(id_p(*make_test_pattern("c")), str("hello_fhatty"), id_p("not_there")));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(10, ping_HIT->load());
    TEST_ASSERT_EQUAL_INT(0, ping_MISS->load());
    delete ping_HIT;
    delete ping_MISS;
    router()->remove(id_p(*make_test_pattern("b")));
    router()->remove(id_p(*make_test_pattern("c")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_read() {
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->write(id_p(*make_test_pattern("x")), str("good")));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->write(id_p(*make_test_pattern("y")), jnt(6)));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->write(id_p(*make_test_pattern("z")), uri(*make_test_pattern("x"))));\
    scheduler()->barrier("waiting for messages in test_read", []() {
      if (auto_loop)
        current_structure->loop();
      return !router()->read(id_p(*make_test_pattern("x")))->is_noobj() &&
             !router()->read(id_p(*make_test_pattern("y")))->is_noobj() &&
             !router()->read(id_p(*make_test_pattern("z")))->is_noobj();
    });
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_STRING("good",
                             router()->read(id_p(*make_test_pattern("x")))->str_value().c_str());
    TEST_ASSERT_EQUAL_INT(Obj::to_int(6)->int_value(), router()->read(id_p(*make_test_pattern("y")))->int_value());
    FOS_TEST_ASSERT_EQUAL_FURI(uri(*make_test_pattern("x"))->uri_value(),
                               router()->read(id_p(*make_test_pattern("z")))->uri_value());
    TEST_ASSERT_EQUAL_STRING("good",
                             router()->read(share(router()->read(id_p(*make_test_pattern("z")))->uri_value()))->
                             str_value().
                             c_str());
    ////// RESET FOR PERSISTENT STRUCTURES
    router()->remove(id_p(*make_test_pattern("x")));
    router()->remove(id_p(*make_test_pattern("y")));
    router()->remove(id_p(*make_test_pattern("z")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_subscribe() {
    //Options::singleton()->log_level(TRACE);
    auto *pings = new atomic_int(0);
    Consumer<Message_p> on_recv_ = [pings](const Message_p &message) {
      FOS_TEST_ASSERT_EQUAL_FURI(Pattern(*make_test_pattern("test")), message->target);
      FOS_TEST_ASSERT_EQUAL_FURI(ID("b/test/case"), message->source);
      TEST_ASSERT_FALSE(message->retain);
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
      .source = "a/test/case", .pattern = *make_test_pattern("test/bad"), .qos = QoS::_1, .onRecv = on_recv_
      })));
    RESPONSE_CODE rc_ = router()->route_subscription(share(Subscription{
      .source = "a/test/case", .pattern = *make_test_pattern("test"), .qos = QoS::_1, .onRecv = on_recv_
    }));
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, rc_);
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->route_message(share(Message{
                        .source = "b/test/case", .target = ID(*make_test_pattern("test")), .payload = Obj::to_bool(true)
                        , .retain =
                        TRANSIENT_MESSAGE
                        })));
    scheduler()->barrier("waiting for messages", [pings]() {
      if (auto_loop)
        current_structure->loop();
      return pings->load() > 0;
    });
    TEST_ASSERT_EQUAL_INT(1, pings->load());
    router()->route_unsubscribe(id_p("a/test/case"), p_p(*make_test_pattern("test")));
    // UNSUBSCRIBE HERE (THUS, NO MORE pings MUTATIONS)
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->route_message(share(Message{
                        .source = "b/test/case", .target = ID(*make_test_pattern("test")), .payload = Obj::to_bool(true)
                        , .retain =
                        TRANSIENT_MESSAGE
                        })));
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    TEST_ASSERT_EQUAL_INT(1, pings->load()); // shouldn't change as subscribe has unsubscribed by now
    delete pings;
    ////// RESET FOR PERSISTENT STRUCTURES
    router()->remove(id_p(*make_test_pattern("test")), id_p("b/test/case"));
    if (auto_loop)
      current_structure->loop();
  }
} // namespace fhatos

#endif
