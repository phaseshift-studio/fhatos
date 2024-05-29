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
    Fluent<> f =
        __(1).plus(2).branch({
            {_.is(_.eq(3)), _.plus(2)},
            {2, 4},
            {_.mult(2), 7}
        }).is(_.eq(5)).mult(_.plus(95));
    FOS_TEST_MESSAGE("%s", f.toString().c_str());
    /*f.forEach<Int>([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });*/
  }

  void test_relational_predicates() {
    List<const Int *> *result;
    result = __({1, 2, 3}).plus(10).is(_.eq(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(12, result->at(0)->value());
    result = __({1, 2, 3}).plus(10).is(_.neq(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(2, result->size());
    TEST_ASSERT_EQUAL_INT(11, result->at(0)->value());
    TEST_ASSERT_EQUAL_INT(13, result->at(1)->value());
    result = __({1, 2, 3}).plus(10).is(_.gt(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(13, result->at(0)->value());
    result = __({1, 2, 3}).plus(10).is(_.gt(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(13, result->at(0)->value());
    result = __({1, 2, 3}).plus(10).is(_.gte(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(2, result->size());
    TEST_ASSERT_EQUAL_INT(12, result->at(0)->value());
    TEST_ASSERT_EQUAL_INT(13, result->at(1)->value());
    result = __({1, 2, 3}).plus(10).is(_.lt(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(11, result->at(0)->value());
    result = __({1, 2, 3}).plus(10).is(_.lte(12)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(2, result->size());
    TEST_ASSERT_EQUAL_INT(11, result->at(0)->value());
    TEST_ASSERT_EQUAL_INT(12, result->at(1)->value());

  }

  void test_plus() {
    const List<const Int *> *result = __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)).toList<Int>();
    TEST_ASSERT_EQUAL_INT(3, result->size());
    TEST_ASSERT_EQUAL_INT(54, result->at(0)->value());
    TEST_ASSERT_EQUAL_INT(50, result->at(1)->value());
    TEST_ASSERT_EQUAL_INT(46, result->at(2)->value());
  }

  void test_where() {
    const List<const Int *> *result = __({1, 2, 3}).plus(10).where(_.is(_.eq(13))).toList<Int>();
    TEST_ASSERT_EQUAL_INT(1, result->size());
    TEST_ASSERT_EQUAL_INT(13, result->front()->value());
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_fluent); //
      FOS_RUN_TEST(test_rec_branch); //
      FOS_RUN_TEST(test_plus); //
      FOS_RUN_TEST(test_where); //
      FOS_RUN_TEST(test_relational_predicates); //
      )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
