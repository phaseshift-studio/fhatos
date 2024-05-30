#include <structure/kernel/f_lang.hpp>
#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

#include <test_fhatos.hpp>
#include <language/fluent.hpp>
#include <language/instructions.hpp>
#include <language/obj.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_fluent() {
    FOS_TEST_MESSAGE("%s", __(19).plus(__(26).plus(5)).plus(5).toString().c_str());
    FOS_TEST_MESSAGE("%s", __(10).plus(_.plus(6).mult(23).plus(
                       _.plus(13).plus(6))).plus(23)
                     .toString()
                     .c_str());

    /*FOS_TEST_MESSAGE("%s", (new Monad<Int>(new Int(32)))->split<Int>(new PlusInst(OBJ_OR_BYTECODE<Int>(10)))
                     ->get()
                     ->toString()
                     .c_str());*/

    __({32, 45}).plus(10).plus(15).forEach<Int>([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });

    FOS_TEST_MESSAGE("=========================\n");

    const Fluent f = __(30).plus(10).plus(15).mult(_.plus(5)); //.mult(__.plus(2).mult(10));
    f.forEach<Int>([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });
  }

  void test_rec_branch() {
    List<const Int *> *result = FOS_TEST(
        __(1).plus(2).bswitch({
            {_.is(_.eq(3)), _.plus(2)},
            {2, 4},
            {_.mult(2), 7}
        }).is(_.eq(5)).mult(_.plus(95))).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(500, result->front()->value());
    /*f.forEach<Int>([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });*/
  }

  void test_dref() {
    const List<const Str *> *result = FOS_TEST_RESULT<Str>(__("fhat").ref("a").plus("os").dref("a"));
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_STRING("fhat", result->front()->value().c_str());
    delete result;
    FOS_DEFAULT_ROUTER::singleton()->clear();
    const List<const Int *> *result2 = FOS_TEST_RESULT<Int>(__(10).ref("a").plus(3).plus(_.dref("a")));
    TEST_ASSERT_EQUAL_INT(1, result2->size());
    TEST_ASSERT_EQUAL_INT(23, result2->front()->value());
    delete result2;
    FOS_DEFAULT_ROUTER::singleton()->clear();
  }

  void test_ref() {
    atomic<int> *counter = new atomic<int>(0);
    List<const Str *> *result = FOS_TEST(__("fhat").ref("a").plus("os")).toList<Str>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_STRING("fhatos", result->front()->value().c_str());
    FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
        .mailbox = nullptr,
        .source = "anon",
        .pattern = Pattern("a"),
        .onRecv = [counter](const Message &message) {
          TEST_ASSERT_EQUAL_INT(0, counter->fetch_add(1));
          TEST_ASSERT_EQUAL_STRING("anon", message.source.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("a", message.target.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("fhat", message.payload->toStr().value().c_str());
        }
    });
    TEST_ASSERT_EQUAL_INT(1, counter->load());
    /////////////////////////////////////////////
    counter->store(0);
    result = FOS_TEST(__({"fhat", "pig"}).bswitch({
        {_.is(_.gt("gonzo")), _.ref("b")},
        {_, _.ref("c")}
    }).plus("y")).toList<Str>();
    TEST_ASSERT_EQUAL_INT(2, result->size());
    result->erase(std::remove_if(result->begin(), result->end(), [](const Str *a)-> bool {
      return a->value() == "fhaty" || a->value() == "pigy";
    }), result->end());
    TEST_ASSERT_EQUAL_INT(0, result->size());
    FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
        .mailbox = nullptr,
        .source = "anon",
        .pattern = Pattern("b"),
        .onRecv = [counter](const Message &message) {
          counter->fetch_add(1);
          TEST_ASSERT_EQUAL_STRING("anon", message.source.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("b", message.target.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("pig", message.payload->toStr().value().c_str());
        }
    });
    FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
        .mailbox = nullptr,
        .source = "anon",
        .pattern = Pattern("c"),
        .onRecv = [counter](const Message &message) {
          counter->fetch_add(1);
          TEST_ASSERT_EQUAL_STRING("anon", message.source.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("c", message.target.toString().c_str());
          TEST_ASSERT_EQUAL_STRING("fhat", message.payload->toStr().value().c_str());
        }
    });
    TEST_ASSERT_EQUAL_INT(2, counter->load());
    FOS_DEFAULT_ROUTER::singleton()->clear();
    delete counter;
    delete result;
  }

  void test_relational_predicates() {
    FOS_CHECK_RESULTS<Int>({12}, __({1, 2, 3}).plus(10).is(_.eq(12)));
    FOS_CHECK_RESULTS<Int>({11, 13}, __({1, 2, 3}).plus(10).is(_.neq(12)));
    FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).is(_.gt(12)));
    FOS_CHECK_RESULTS<Int>({12, 13}, __({1, 2, 3}).plus(10).is(_.gte(12)));
    FOS_CHECK_RESULTS<Int>({11}, __({1, 2, 3}).plus(10).is(_.lt(12)));
    FOS_CHECK_RESULTS<Int>({11, 12}, __({1, 2, 3}).plus(10).is(_.lte(12)));
  }

  void test_plus() {
    const List<const Int *> *result = FOS_TEST_RESULT<Int>(__({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));
    TEST_ASSERT_EQUAL_INT(3, result->size());
    TEST_ASSERT_EQUAL_INT(54, result->at(0)->value());
    TEST_ASSERT_EQUAL_INT(50, result->at(1)->value());
    TEST_ASSERT_EQUAL_INT(46, result->at(2)->value());
  }

  void test_where() {
    const List<const Int *> *result = FOS_TEST_RESULT<Int>(__({1, 2, 3}).plus(10).where(_.is(_.eq(13))));
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(13, result->front()->value());
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_fluent); //
      FOS_RUN_TEST(test_rec_branch); //
      FOS_RUN_TEST(test_ref); //
      FOS_RUN_TEST(test_dref); //
      FOS_RUN_TEST(test_plus); //
      FOS_RUN_TEST(test_where); //
      FOS_RUN_TEST(test_relational_predicates); //
      )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
