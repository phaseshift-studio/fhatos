#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

// #include <atomic>
#include <language/fluent.hpp>
#include <language/instructions.hpp>
#include <test_fhatos.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_fluent() {
    FOS_PRINT_FLUENT(__(19).plus(__().plus(5)).plus(5));
    FOS_PRINT_FLUENT(__(10).plus(_.plus(6).mult(23).plus(_.plus(13).plus(6))).plus(23));

    /*FOS_CHECK_RESULTS<Str>(
            {"acdzx", "bbcddx"},
            __({"a", "bb"})
                .plus("c")
                .bswitch({{_.is(_.eq("ac")), _.plus("d").bswitch({{_.is(_.gte("a")), _.plus("z")}, {_, _}})},
                          {_.is(_.gte("www")), _.plus("yy")},
                          {_, _.plus("dd")}})
                .is(_.lt("xxx"))
                .plus("x"));*/

    __({32, 45}).plus(10).plus(15).forEach<Int>([](const Int_p e) { FOS_TEST_MESSAGE("=>%s", e->toString().c_str()); });

    FOS_TEST_MESSAGE("=========================\n");

    const Fluent f = __(30).plus(10).plus(15).mult(_.plus(5)); //.mult(__.plus(2).mult(10));
    f.forEach<Obj>([](Obj_p e) { FOS_TEST_MESSAGE("=>%s", e->toString().c_str()); });
  }

  /*void test_select() {
    FOS_CHECK_RESULTS<Rec>({Rec({{Uri::of("a"), Int::of(1)}, {Uri::of("b"), Int::of(2)}, {Uri::of("c"), Int::of(3)}})},
                           __(1).ref("a").plus(1).ref("b").plus(1).ref("c").select({Uri("a"), Uri("b"), Uri("c")}));

    FOS_CHECK_RESULTS<Rec>(
        {Rec({{Uri::of("a"), Int::of(2)}, {Uri::of("b"), Int::of(4)}, {Uri::of("c"), Int::of(6)}})},
        __(1).ref("a").plus(1).ref("b").plus(1).ref("c").select(Rec({{Uri::of("a"), Int::of(_.plus(1))},
                                                                     {Uri::of("b"), Int::of(_.plus(2))},
                                                                     {Uri::of("c"), Int::of(_.plus(3))}})));
  }*/
  /*
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
*/

  void test_ref_dref() {
    FOS_CHECK_RESULTS<Int>({2}, __(1).to(u("a")).plus(_.from(u("a"))), {{u("a"), 1}});
    FOS_CHECK_RESULTS<Int>({23}, __(10).to(u("a")).plus(3).plus(_.from(u("a"))), {{u("a"), 10}});
    FOS_CHECK_RESULTS<Str>({"fhatos"}, __("fhat").to(u("a")).plus("os"), {{u("a"), "fhat"}});
    /* FOS_CHECK_RESULTS<Str>(
         {"fhaty", "pigy"},
         __({"fhat", "pig"}).bswitch({{_.is(_.gt("gonzo")), _.ref(u("b"))}, {_, _.ref(u("c"))}}).plus("y"),
         {{u("b"), "pig"}, {u("c"), "fhat"}});*/
    FOS_DEFAULT_ROUTER::singleton()->clear();
  }

  void test_relational_predicates() {
    FOS_CHECK_RESULTS<Int>({1}, __(1).is(_.eq(1)));
    FOS_CHECK_RESULTS<Int>({}, __(1).is(_.neq(1)));
    FOS_CHECK_RESULTS<Int>({12}, __({1, 2, 3}).plus(10).is(_.eq(12)));
    FOS_CHECK_RESULTS<Int>({11, 13}, __({1, 2, 3}).plus(10).is(_.neq(12)));
    FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).is(_.gt(12)));
    FOS_CHECK_RESULTS<Int>({12, 13}, __({1, 2, 3}).plus(10).is(_.gte(12)));
    FOS_CHECK_RESULTS<Int>({11}, __({1, 2, 3}).plus(10).is(_.lt(12)));
    FOS_CHECK_RESULTS<Int>({11, 12}, __({1, 2, 3}).plus(10).is(_.lte(12)));
  }

  void test_plus() {
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(false));
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(true));
    FOS_CHECK_RESULTS<Bool>({false, true, false}, __({false, true, false}).plus(false));
    //
    FOS_CHECK_RESULTS<Int>({3}, __(1).plus(2));
    FOS_CHECK_RESULTS<Int>({54, 50, 46}, __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));
    //
    FOS_CHECK_RESULTS<Real>({46.5f}, __(1.121f).plus(10.002f).plus(_).plus(_.plus(2.0f)));
    FOS_CHECK_RESULTS<Real>({54.4f, 50.4f, 46.4f}, __({1.05f, 2.05f, 3.05f}).plus(10.05f).plus(_).plus(_.plus(2.0f)));
    //
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("/b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b/")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b")));
    //
    FOS_CHECK_RESULTS<Str>({"http://fhatos.org/a/b", "fhat.pig/a/b"},
                           __(List<Obj>{"http://fhatos.org", "fhat.pig"}).plus("/a").plus("/b"));
    //
    FOS_CHECK_RESULTS<Rec>({{{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}}},
                           __(Rec{{"a", 1}}).plus({{"b", 2}}).plus({{"c", 3}, {"d", 4}}));
  }

  void test_mult() {
    // FOS_CHECK_RESULTS<Rec>({Rec{{21, 10}, {48, 36}}}, __(Rec{{3, 2}, {6, 4}}).mult(Rec{{7, 5}, {8, 9}}));
  }

  /* void test_where() { FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).where(_.is(_.eq(13)))); }*/

  void test_define_as_type() {
    FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).define(u("/int/nat"), _.is(_.gt(0))).as(u("/int/nat")),
                           {{u("/int/nat"), _.is(_.gt(0))}});
    FOS_CHECK_RESULTS<Int>({1}, __(1).define(u("/int/nat"), _.is(_.gt(0))), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).as(u("/int/nat")), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    FOS_CHECK_RESULTS<Uri>({u("/int/nat")}, __(1).as(u("/int/nat")).type(), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    FOS_CHECK_RESULTS<Int>({Int(6, "/int/nat2")},
                           __(1)
                               .as(u("nat"))
                               .plus(Int(2, "nat"))
                               .to(u("x"))
                               .plus(Int(3, "nat"))
                               .type()
                               .from(_)
                               .define(u("/int/nat2"), _.is(_.gt(0)))
                               .from(u("x"))
                               .as(u("/int/nat2"))
                               .mult(Int(2, "/int/nat2")),
                           {{u("/int/nat"), _.is(_.gt(0))}, {u("/int/nat2"), _.is(_.gt(0))}, {u("x"), Int(3, "nat")}},
                           false);
  }


  FOS_RUN_TESTS( //
      Obj::Types<>::addToCache(share(fURI("/int/nat")), Insts::NO_OP_BCODE()); //
      Obj::Types<>::addToCache(share(fURI("/int/nat2")), Insts::NO_OP_BCODE()); //
      FOS_RUN_TEST(test_ref_dref); //
      FOS_RUN_TEST(test_plus); //
      // FOS_RUN_TEST(test_fluent); //
      FOS_RUN_TEST(test_mult); //
      FOS_RUN_TEST(test_relational_predicates); //
      // FOS_RUN_TEST(test_select); //
      FOS_RUN_TEST(test_define_as_type); //
  )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
