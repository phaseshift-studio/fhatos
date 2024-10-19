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
#define FOS_DEPLOY_EXT
#include <test_fhatos.hpp>

#include "../../build/_deps/unity-src/src/unity.h"

namespace fhatos {
  inline Structure_p current_structure;
  inline bool auto_loop;
  inline fURI_p prefix;

  inline fURI_p make_test_pattern(const char *suffix) { return furi_p(prefix->extend(suffix)); }

  inline void begin_test_structure(const Structure_p &test_structure, const bool autoloop = true) {
    current_structure = test_structure;
    auto_loop = autoloop;;
    prefix = furi_p(current_structure->pattern()->retract());
    TEST_ASSERT_TRUE(make_test_pattern("+")->equals(*current_structure->pattern()) ||
      make_test_pattern("#")->equals(*current_structure->pattern()));
    LOG(INFO, "!yStarting!! test_base_structure !yon !g%s!y using !g%s!y pattern\n",
        current_structure->pattern()->toString().c_str(), make_test_pattern("+")->toString().c_str());
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
    const Subscription_p subscription_HIT = subscription_p(
      "tester_HIT",
      *make_test_pattern("+"),
      Insts::to_bcode([ping_HIT](const Message_p &message) {
        LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(*make_test_pattern("b"), message->target);
        TEST_ASSERT_TRUE_MESSAGE(message->payload->is_rec(),
                                 (string("Expected rec but received ") + message->payload->type()->toString()).c_str());
        FL_INT_TYPE payload_int = message->payload->rec_value()->at(str("hello_fhatty"))->int_value();
        TEST_ASSERT_EQUAL_INT(payload_int, ping_HIT->load());
        // TEST_ASSERT_TRUE(message->retain);
        ping_HIT->store(ping_HIT->load() + 1);
      }));
    const Subscription_p subscription_MISS =
        subscription_p("tester_MISS",
                       *make_test_pattern("c"),
                       Insts::to_bcode([ping_MISS](const Message_p &message) {
                         ping_MISS->store(ping_MISS->load() + 1);
                         LOG(INFO, "Received message from subscriber: %s\n", message->toString().c_str());
                         TEST_FAIL_MESSAGE((string("Subscription ") + make_test_pattern("c")->toString() +
                             " does not match payload target:" + message->target.toString())
                           .c_str());
                       }));
    router()->route_subscription(subscription_HIT);
    router()->route_subscription(subscription_MISS);
    if (auto_loop)
      current_structure->loop();
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/b/c"), jnt(10)));
    // FOS_TEST_EXCEPTION_CXX(router()->write(id_p(*make_test_pattern("b/c")), str("hello_fhatty"), id_p("aus")));
    // FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/b/c"), str("hello_fhatty"), id_p("aus")));
    //  TODO: FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/"), str("hello_fhatty"), id_p("aus")));
    // FOS_TEST_EXCEPTION_CXX(router()->write(id_p(*make_test_pattern("a/a/b/c")), str("hello_fhatty"), id_p("aus")));
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/"), str("hello_fhatty")));
    for (int i = 0; i < 10; i++) {
      TEST_ASSERT_EQUAL_INT(i, ping_HIT->load());
      router()->write(id_p(*make_test_pattern("b")), rec({{str("hello_fhatty"), jnt(i)}}));
      if (auto_loop)
        current_structure->loop();
      scheduler()->barrier("waiting_for_messages/test_write/1", [ping_HIT, i]() {
        if (auto_loop)
          current_structure->loop();
        return ping_HIT->load() > i;
      });
    }
    router()->route_unsubscribe(id_p("tester_HIT"), p_p(*make_test_pattern("+")));
    if (auto_loop)
      current_structure->loop();
    // should not cause an exception due to str != rec as no subscription exists
    router()->write(id_p(*make_test_pattern("b")), str("hello_fhatty"));
    scheduler()->barrier("waiting_for_messages/test_write/2", [ping_HIT]() {
      if (auto_loop)
        current_structure->loop();
      return ping_HIT->load() > 9;
    });
    TEST_ASSERT_EQUAL_INT(10, ping_HIT->load());
    TEST_ASSERT_EQUAL_INT(0, ping_MISS->load());
    router()->route_unsubscribe(id_p("tester_MISS"), p_p(*make_test_pattern("c")));
    if (auto_loop)
      current_structure->loop();
    router()->write(id_p(*make_test_pattern("c")), str("hello_fhatty"));
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
    router()->write(id_p(*make_test_pattern("x")), str("good"));
    router()->write(id_p(*make_test_pattern("y")), jnt(6));
    router()->write(id_p(*make_test_pattern("z")), vri(*make_test_pattern("x")));
    scheduler()->barrier("waiting_for_messages/test_read/1", []() {
      if (auto_loop)
        current_structure->loop();
      return !router()->read(id_p(*make_test_pattern("x")))->is_noobj() &&
             !router()->read(id_p(*make_test_pattern("y")))->is_noobj() &&
             !router()->read(id_p(*make_test_pattern("z")))->is_noobj();
    });
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_STRING("good", router()->read(id_p(*make_test_pattern("x")))->str_value().c_str());
    TEST_ASSERT_EQUAL_INT(Obj::to_int(6)->int_value(), router()->read(id_p(*make_test_pattern("y")))->int_value());
    FOS_TEST_ASSERT_EQUAL_FURI(vri(*make_test_pattern("x"))->uri_value(),
                               router()->read(id_p(*make_test_pattern("z")))->uri_value());
    TEST_ASSERT_EQUAL_STRING(
      "good", router()->read(share(router()->read(id_p(*make_test_pattern("z")))->uri_value()))->str_value().c_str());
    ////// RESET FOR PERSISTENT STRUCTURES
    router()->remove(id_p(*make_test_pattern("x")));
    router()->remove(id_p(*make_test_pattern("y")));
    router()->remove(id_p(*make_test_pattern("z")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_data_types() {
    current_structure->recv_subscription(subscription_p(
      "test_data_types", *make_test_pattern("abc"),
      OBJ_PARSER(string("get(payload).print(_).-<(["
        "type().is(eq(/type/bool/)) => is(eq(true)),"
        "type().is(eq(/type/int/)) => is(eq(10)),"
        "type().is(eq(/type/real/)) => is(eq(15.5)),"
        "type().is(eq(/type/str/)) => is(eq('here')),"
        "type().is(eq(/type/uri/)) => "
        "is(eq(<http://fhatos.org>))]).merge(1).count().is(eq(0)).error('wrong parse')"))));
    //"type().is(eq(/type/lst/)) => is(eq([1,2,3])),"
    //"type().is(eq(/type/rec/)) => is(eq([1=>2,3=>4]))])
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), dool(true), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), jnt(10), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), real(15.5), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), str("here"), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), uri("http://fhatos.org"), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), lst({jnt(1), jnt(2), jnt(3)}), false));
    // current_structure->recv_publication(message_p(*make_test_pattern("abc"), rec({{jnt(1), jnt(2)}, {jnt(3),
    // jnt(4)}}),
    //                                             false));
    if (auto_loop)
      current_structure->loop();
    current_structure->recv_unsubscribe(id_p("test_data_types"), id_p(*make_test_pattern("abc")));
    if (auto_loop)
      current_structure->loop();
    current_structure->remove(id_p(*make_test_pattern("abc")));
  }

  void test_subscribe() {
    // Options::singleton()->log_level(TRACE);
    auto *pings = new atomic_int(0);
    const BCode_p on_recv = Insts::to_bcode([pings](const Message_p &message) {
      FOS_TEST_ASSERT_EQUAL_FURI(Pattern(*make_test_pattern("test")), message->target);
      TEST_ASSERT_FALSE(message->retain);
      if (message->payload->is_bool()) {
        TEST_ASSERT_TRUE(message->payload->bool_value());
        pings->store(pings->load() + 1);
        TEST_ASSERT_EQUAL(1, pings->load());
      }
    });
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(0, pings->load());
    FOS_TEST_EXCEPTION_CXX(router()->route_subscription(subscription_p("a/test/case","a/test/bad",on_recv)));
    router()->route_subscription(subscription_p("a/test/case", *make_test_pattern("test"), on_recv));
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    router()->write(id_p(*make_test_pattern("test")), Obj::to_bool(true), TRANSIENT_MESSAGE);
    scheduler()->barrier("waiting_for_messages", [pings]() {
      if (auto_loop)
        current_structure->loop();
      return pings->load() > 0;
    });
    TEST_ASSERT_EQUAL_INT(1, pings->load());
    router()->route_unsubscribe(id_p("a/test/case"), p_p(*make_test_pattern("test")));
    // UNSUBSCRIBE HERE (THUS, NO MORE pings MUTATIONS)
    if (auto_loop)
      current_structure->loop();
    router()->write(id_p(*make_test_pattern("test")), Obj::to_bool(true), TRANSIENT_MESSAGE);
    if (auto_loop)
      current_structure->loop(); // TODO: automatic for particular SType?
    TEST_ASSERT_EQUAL_INT(1, pings->load()); // shouldn't change as subscribe has unsubscribed by now
    delete pings;
    ////// RESET FOR PERSISTENT STRUCTURES
    router()->remove(id_p(*make_test_pattern("test")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_patterned_reads() {
    current_structure->write(id_p(*make_test_pattern("a")), str("a"), true);
    current_structure->write(id_p(*make_test_pattern("b")), str("b"), true);
    current_structure->write(id_p(*make_test_pattern("c")), str("c"), true);
    current_structure->write(id_p(*make_test_pattern("d")), str("d"), true);
    if (auto_loop)
      current_structure->loop();
    Objs_p objs = current_structure->read(make_test_pattern("+"));
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_TRUE(objs->is_objs());
    TEST_ASSERT_EQUAL_INT(4, objs->objs_value()->size());
    int counter = 0;
    for (const Str_p &furi: {str("a"), str("b"), str("c"), str("d")}) {
      TEST_ASSERT_EQUAL_INT(
        1, std::count_if(objs->objs_value()->begin(), objs->objs_value()->end(), [furi](const Obj_p &obj) {
          FOS_TEST_ASSERT_MATCH_FURI(*obj->type(), *STR_FURI);
          TEST_ASSERT_TRUE(obj->is_str());
          return obj->str_value() == furi->str_value();
          }));
      counter++;
    }
    TEST_ASSERT_EQUAL_INT(4, counter);
    ////// RESET FOR PERSISTENT STRUCTURES
    current_structure->remove(id_p(*make_test_pattern("a")));
    current_structure->remove(id_p(*make_test_pattern("b")));
    current_structure->remove(id_p(*make_test_pattern("c")));
    current_structure->remove(id_p(*make_test_pattern("d")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_ided_reads() {
    current_structure->write(id_p(*make_test_pattern("a")), str("a"), true);
    current_structure->write(id_p(*make_test_pattern("b")), str("b"), true);
    current_structure->write(id_p(*make_test_pattern("c")), str("c"), true);
    current_structure->write(id_p(*make_test_pattern("d")), str("d"), true);
    if (auto_loop)
      current_structure->loop();
    int counter = 0;
    for (const fURI &furi:
         {*make_test_pattern("a"), *make_test_pattern("b"), *make_test_pattern("c"), *make_test_pattern("d")}) {
      const Obj_p obj = current_structure->read(id_p(furi));
      if (auto_loop)
        current_structure->loop();
      TEST_ASSERT_TRUE(obj->is_str());
      TEST_ASSERT_EQUAL_STRING(furi.name().c_str(), obj->str_value().c_str());
      counter++;
    }
    TEST_ASSERT_EQUAL_INT(4, counter);
    ////// RESET FOR PERSISTENT STRUCTURES
    current_structure->remove(id_p(*make_test_pattern("a")));
    current_structure->remove(id_p(*make_test_pattern("b")));
    current_structure->remove(id_p(*make_test_pattern("c")));
    current_structure->remove(id_p(*make_test_pattern("d")));
    if (auto_loop)
      current_structure->loop();
  }

  void test_embedding() {
    //////////////////////////////////////////////////////////
    ///////////////////////////////////////// SIMPLE EMBEDDING
    //////////////////////////////////////////////////////////
    process("%s -> [./aa/dd/aaa=>1,./aa/bb=>2]", make_test_pattern("x/")->toString().c_str());
    const string A = make_test_pattern("x/aa/dd/aaa")->toString();
    const string B = make_test_pattern("x/aa/bb")->toString();
    TEST_ASSERT_EQUAL_INT(1, process("*<%s>.is(eq(1))", A.c_str())->objs_value()->size());
    TEST_ASSERT_EQUAL_INT(0, process("*%s.is(eq(2))", A.c_str())->objs_value()->size());
    TEST_ASSERT_EQUAL_INT(0, process("*%s.is(eq(1))", B.c_str())->objs_value()->size());
    TEST_ASSERT_EQUAL_INT(1, process("*<%s>.is(eq(2))", B.c_str())->objs_value()->size());
    ////// RESET FOR PERSISTENT STRUCTURES
    process("%s -> noobj", A.c_str());
    process("%s -> noobj", B.c_str());
    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////// COMPLEX EMBEDDING
    ///////////////////////////////////////////////////////////
    TEST_ASSERT_EQUAL_INT(1, process("%s -> [./a/ => "
                            "         [./b => 1], "
                            "        ./aa/ => "
                            "         [./bb => '2', "
                            "          ./cc => |<3>, "
                            "          ./dd/ => "
                            "            [./aaa => true, "
                            "             ./bbb => 5.0], "
                            "          ./ee/ => [6,7,8]], "
                            "        ./aaa => 9, "
                            "        'aaaa' => 'the number 10']",
                            make_test_pattern("x/")->toString().c_str())
                          ->objs_value()
                          ->size());
    FOS_TEST_OBJ_EQUAL(jnt(1), current_structure->read(id_p(*make_test_pattern("x/a/b"))));
    FOS_TEST_OBJ_EQUAL(str("2"), current_structure->read(id_p(*make_test_pattern("x/aa/bb"))));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("3"), current_structure->read(id_p(*make_test_pattern("x/aa/cc")))->uri_value());
    TEST_ASSERT_TRUE(current_structure->read(id_p(*make_test_pattern("x/aa/dd/aaa")))->bool_value());
    FOS_TEST_OBJ_EQUAL(real(5.0f), current_structure->read(id_p(*make_test_pattern("x/aa/dd/bbb"))));
    FOS_TEST_OBJ_EQUAL(jnt(6), current_structure->read(id_p(*make_test_pattern("x/aa/ee/0"))));
    FOS_TEST_OBJ_EQUAL(jnt(7), current_structure->read(id_p(*make_test_pattern("x/aa/ee/1"))));
    FOS_TEST_OBJ_EQUAL(jnt(8), current_structure->read(id_p(*make_test_pattern("x/aa/ee/2"))));
    FOS_TEST_OBJ_EQUAL(jnt(9), current_structure->read(id_p(*make_test_pattern("x/aaa"))));
    FOS_TEST_OBJ_EQUAL(str("the number 10"),
                       current_structure->read(id_p(*make_test_pattern("x/0")))->rec_value()->at(str("aaaa")));

    const Objs_p objs2 = process("*%s", make_test_pattern("x/#")->toString().c_str());
    TEST_ASSERT_EQUAL_INT(10, objs2->objs_value()->size());
    ////// RESET FOR PERSISTENT STRUCTURES
    current_structure->remove(id_p(*make_test_pattern("x/0")));
    current_structure->remove(id_p(*make_test_pattern("x/aaa")));
    for (int i = 1; i < 4; i++) {
      const Pattern_p p = p_p(make_test_pattern("x/")->extend(StringHelper::repeat(i, "+/")));
      LOG(DEBUG, "!yRemove pattern!!: !b%s!!\n", p->toString().c_str());
      const Rec_p rec3 = current_structure->read(p);
      for (const auto &[key, value]: *rec3->rec_value()) {
        current_structure->remove(id_p(key->uri_value()));
      }
      if (auto_loop)
        current_structure->loop();
    }
    if (auto_loop)
      current_structure->loop();
    TEST_ASSERT_EQUAL_INT(0, process("*%s", make_test_pattern("x/#")->toString().c_str())->objs_value()->size());
  }

  void test_from_at() {
    fURI_p idA1 = make_test_pattern("a1");
    fURI_p idA2 = make_test_pattern("a2");
    process("%s -> 42", idA1->toString().c_str());
    Int_p i = process("*%s", idA1->toString().c_str())->objs_value()->front();
    TEST_ASSERT_EQUAL_INT(42, i->int_value());
    TEST_ASSERT_NULL(i->id());
    /////
    Int_p j = process("@%s", idA1->toString().c_str())->objs_value()->front();
    TEST_ASSERT_EQUAL_INT(42, j->int_value());
    FOS_TEST_ASSERT_EQUAL_FURI(*idA1, *j->id());
    const Int k = *j + (Int(2));
    TEST_ASSERT_EQUAL_INT(44, k.int_value());
    TEST_ASSERT_EQUAL_INT(42, i->int_value());
    const Int_p l = process("@%s", idA1->toString().c_str())->objs_value()->front();
    TEST_ASSERT_EQUAL_INT(44, l->int_value());
    FOS_TEST_ASSERT_EQUAL_FURI(*idA1, *l->id());
  }
} // namespace fhatos

#endif
