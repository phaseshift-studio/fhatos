#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

#include <atomic>
#include <language/fluent.hpp>
#include <language/obj.hpp>
#include <test_fhatos.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_fluent() {
    FOS_TEST_MESSAGE("%s", __(19).plus(__(26).plus(5)).plus(5).toString().c_str());
    FOS_TEST_MESSAGE("%s", __(10).plus(_.plus(6).mult(23).plus(_.plus(13).plus(6))).plus(23).toString().c_str());

    /*FOS_TEST_MESSAGE("%s", (new Monad<Int>(new Int(32)))->split<Int>(new PlusInst(OBJ_OR_BYTECODE<Int>(10)))
                     ->get()
                     ->toString()
                     .c_str());*/

    __({32, 45}).plus(10).plus(15).forEach<Int>([](const Int *e) { FOS_TEST_MESSAGE("=>%s", e->toString().c_str()); });

    FOS_TEST_MESSAGE("=========================\n");

    const Fluent f = __(30).plus(10).plus(15).mult(_.plus(5)); //.mult(__.plus(2).mult(10));
    f.forEach<Int>([](const Int *e) { FOS_TEST_MESSAGE("=>%s", e->toString().c_str()); });
  }

  void test_select() {
    FOS_CHECK_RESULTS<Rec>({
      Rec({
        {new Uri("a"), new Int(1)},
        {new Uri("b"),new Int(2)},
        {new Uri("c"),new Int(3)}})},
      __(1).ref("a").plus(1).ref("b").plus(1).ref("c").select({Uri("a"),Uri("b"),Uri("c")}));

    FOS_CHECK_RESULTS<Rec>({
    Rec({
      {new Uri("a"), new Int(2)},
      {new Uri("b"),new Int(4)},
      {new Uri("c"),new Int(6)}})},
    __(1).ref("a").plus(1).ref("b").plus(1).ref("c").select({{Uri("a"),_.plus(1)},{Uri("b"),_.plus(2)},{Uri("c"),_.plus(3)}}));
  }

  void test_rec_branch() {
    FOS_CHECK_RESULTS<Int>(
        {500},
        __(1).plus(2).bswitch({{_.is(_.eq(3)), _.plus(2)}, {2, 4}, {_.mult(2), 7}}).is(_.eq(5)).mult(_.plus(95)));

    FOS_CHECK_RESULTS<Str>(
        {"acdx", "bbcddx"},
        __({"a", "bb"})
            .plus("c")
            .bswitch({{_.is(_.eq("ac")), _.plus("d")}, {_.is(_.gte("www")), _.plus("yy")}, {_, _.plus("dd")}})
            .is(_.lt("xxx"))
            .plus("x"));

    FOS_CHECK_RESULTS<Str>(
        {"acdzx", "bbcddx"},
        __({"a", "bb"})
            .plus("c")
            .bswitch({{_.is(_.eq("ac")), _.plus("d").bswitch({{_.is(_.gte("a")), _.plus("z")}, {_, _}})},
                      {_.is(_.gte("www")), _.plus("yy")},
                      {_, _.plus("dd")}})
            .is(_.lt("xxx"))
            .plus("x"));
  }


  void test_count() {
    FOS_CHECK_RESULTS<Int>({3}, __({"fhat", "os", "pig"}).plus(".").count());
    FOS_CHECK_RESULTS<Int>({1}, __({"fhat", "os", "pig"}).is(_.eq("pig")).plus(";").count());
    // TODO: FOS_CHECK_RESULTS<Int>({Int(0)}, __({"fhat", "os", "pig"}).is(_.eq("blah")).count());
  }

  void test_dref() {
    FOS_CHECK_RESULTS<Str>({"fhat"}, __("fhat").ref("a").plus("os").dref("a"), {{Uri("a"), new Str("fhat")}});
    FOS_CHECK_RESULTS<Int>({23}, __(10).ref("a").plus(3).plus(_.dref("a")), {{Uri("a"), new Int(10)}});
  }

  void test_ref() {

    FOS_CHECK_RESULTS<Str>({"fhatos"}, __("fhat").ref("a").plus("os"), {{Uri("a"), new Str("fhat")}});
    FOS_CHECK_RESULTS<Str>({"fhaty", "pigy"},
                           __({"fhat", "pig"}).bswitch({{_.is(_.gt("gonzo")), _.ref("b")}, {_, _.ref("c")}}).plus("y"),
                           {{Uri("b"), new Str("pig")}, {Uri("c"), new Str("fhat")}});
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

  void test_mult() {
    FOS_CHECK_RESULTS<Int>({54, 50, 46}, __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));
    // FOS_CHECK_RESULTS<Real>({54.0f, 50.0f, 46.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).plus(_).plus(_.plus(2.0f)));
    FOS_CHECK_RESULTS<Rec>({{{new Int(21), new Int(10)}, {new Int(48), new Int(36)}}},
                           __({{3, 2}, {6, 4}}).mult({{7, 5}, {8, 9}}));
  }

  void test_where() { FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).where(_.is(_.eq(13)))); }

  void test_define_and_type() {
    FOS_CHECK_RESULTS<Int>({*Int(1).cast("nat")}, __(1).define("nat", _.is(_.gt(0))).as(fURI("nat")),
                           {{fURI("nat"), _.is(_.gt(0)).bcode.get()}});
    FOS_CHECK_RESULTS<Int>({1}, __(1).define("nat", _.is(_.gt(0))), {{fURI("nat"), _.is(_.gt(0)).bcode.get()}}, false);
    FOS_CHECK_RESULTS<Int>({*Int(1).cast("nat")}, __(1).as(fURI("nat")), {{fURI("nat"), _.is(_.gt(0)).bcode.get()}},
                           false);
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_fluent); //
      FOS_RUN_TEST(test_select); //
      FOS_RUN_TEST(test_rec_branch); //
      FOS_RUN_TEST(test_count); //
      FOS_RUN_TEST(test_ref); //
      FOS_RUN_TEST(test_dref); //
      FOS_RUN_TEST(test_plus); //
      FOS_RUN_TEST(test_mult); //
      FOS_RUN_TEST(test_where); //
      FOS_RUN_TEST(test_define_and_type); //
      FOS_RUN_TEST(test_relational_predicates); //
  )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
