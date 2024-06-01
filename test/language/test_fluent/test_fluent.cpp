#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

#include <test_fhatos.hpp>
#include <language/fluent.hpp>
#include <language/obj.hpp>
#include <atomic>

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
    FOS_CHECK_RESULTS<Int>(
        {500}, __(1).plus(2).bswitch({
            {_.is(_.eq(3)), _.plus(2)},
            {2, 4},
            {_.mult(2), 7}
        }).is(_.eq(5)).mult(_.plus(95)));
  }

  void test_dref() {
    FOS_CHECK_RESULTS<Str>({"fhat"}, __("fhat").ref("a").plus("os").dref("a"),
                           {{Uri("a"), new Str("fhat")}});
    FOS_CHECK_RESULTS<Int>({23}, __(10).ref("a").plus(3).plus(_.dref("a")),
                           {{Uri("a"), new Int(10)}});
  }

  void test_ref() {

    FOS_CHECK_RESULTS<Str>({"fhatos"}, __("fhat").ref("a").plus("os"),
                           {
                               {Uri("a"), new Str("fhat")}});
    FOS_CHECK_RESULTS<Str>({"fhaty", "pigy"}, __({"fhat", "pig"}).bswitch({
                               {_.is(_.gt("gonzo")), _.ref("b")},
                               {_, _.ref("c")}}).plus("y"),
                           {
                               {Uri("b"), new Str("pig")},
                               {Uri("c"), new Str("fhat")}});
    FOS_DEFAULT_ROUTER::singleton()->clear();
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
    FOS_CHECK_RESULTS<Int>({3}, __(1).plus(2));
    FOS_CHECK_RESULTS<Int>({54, 50, 46}, __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));

  }

  void test_where() {
    FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).where(_.is(_.eq(13))));
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
