#include <process/ptype/native/scheduler.hpp>
#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_to_from() {
    FOS_CHECK_RESULTS<Int>({2}, __(1).to(u("a")).plus(_.from(u("a"))), {{u("a"), 1}});
    FOS_CHECK_RESULTS<Int>({23}, __(10).to(u("b")).plus(3).plus(_.from(u("b"))), {{u("b"), 10}});
    FOS_CHECK_RESULTS<Str>({"fhatos"}, __("fhat").to(u("c")).plus("os"), {{u("c"), "fhat"}});
    /*FOS_CHECK_RESULTS<Str>(
        {"fhaty", "pigy"},
        __(List<Obj>{"fhat", "pig"})
            .bswitch({{*_.is(_.gt("gonzo"))._bcode, *_.to(u("b"))._bcode}, {*_._bcode, *_.to(u("c"))._bcode}})
            .plus("y"),
        {{u("b"), "pig"}, {u("c"), "fhat"}});*/
  }

  void test_relational_predicates() {
    //// INT
    FOS_CHECK_RESULTS<Int>({1}, __(1).is(_.eq(1)));
    FOS_CHECK_RESULTS<Int>({}, __(1).is(_.neq(1)));
    FOS_CHECK_RESULTS<Int>({12}, __({1, 2, 3}).plus(10).is(_.eq(12)));
    FOS_CHECK_RESULTS<Int>({11, 13}, __({1, 2, 3}).plus(10).is(_.neq(12)));
    FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).is(_.gt(12)));
    FOS_CHECK_RESULTS<Int>({12, 13}, __({1, 2, 3}).plus(10).is(_.gte(12)));
    FOS_CHECK_RESULTS<Int>({11}, __({1, 2, 3}).plus(10).is(_.lt(12)));
    FOS_CHECK_RESULTS<Int>({11, 12}, __({1, 2, 3}).plus(10).is(_.lte(12)));
    //// REAL
    FOS_CHECK_RESULTS<Int>({1.0f}, __(1.0f).is(_.eq(1.0f)));
    FOS_CHECK_RESULTS<Int>({}, __(1.0f).is(_.neq(1.0f)));
    FOS_CHECK_RESULTS<Int>({12.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.eq(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f, 13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.neq(12.0f)));
    FOS_CHECK_RESULTS<Int>({13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gt(12.0f)));
    FOS_CHECK_RESULTS<Int>({12.0f, 13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gte(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lt(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f, 12.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lte(12.0f)));
    //// STR
    FOS_CHECK_RESULTS<Int>({"1"}, __("1").is(_.eq("1")));
    FOS_CHECK_RESULTS<Int>({}, __("1").is(_.neq("1")));
    FOS_CHECK_RESULTS<Int>({"20"}, __({"1", "2", "3"}).plus("0").is(_.eq("20")));
    FOS_CHECK_RESULTS<Int>({"10", "30"}, __({"1", "2", "3"}).plus("0").is(_.neq("20")));
    FOS_CHECK_RESULTS<Int>({"30"}, __({"1", "2", "3"}).plus("0").is(_.gt("20")));
    FOS_CHECK_RESULTS<Int>({"20", "30"}, __({"1", "2", "3"}).plus("0").is(_.gte("20")));
    FOS_CHECK_RESULTS<Int>({"10"}, __({"1", "2", "3"}).plus("0").is(_.lt("20")));
    FOS_CHECK_RESULTS<Int>({"10", "20"}, __({"1", "2", "3"}).plus("0").is(_.lte("20")));
  }

  void test_plus() {
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(false));
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(true));
    FOS_CHECK_RESULTS<Bool>({false, true, false}, __(List<Obj>{false, true, false}).plus(false));
    //
    FOS_CHECK_RESULTS<Int>({3}, __(1).plus(2));
    FOS_CHECK_RESULTS<Int>({54, 50, 46}, __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));
    //
    FOS_CHECK_RESULTS<Real>({46.5f}, __(1.121f).plus(10.002f).plus(_).plus(_.plus(2.0f)));
    FOS_CHECK_RESULTS<Real>({54.4f, 50.4f, 46.4f}, __({1.05f, 2.05f, 3.05f}).plus(10.05f).plus(_).plus(_.plus(2.0f)));
    //
    /* FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("b")));
     FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("/b")));
     FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//b")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b")));
     FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b/")));
     FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//../b/")},
                            __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b/")));
     FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//../b/.")},
                            __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b")).plus(u(".")));
     //
     FOS_CHECK_RESULTS<Str>({"http://fhatos.org/a/b", "fhat.pig/a/b"},
                            __({"http://fhatos.org", "fhat.pig"}).plus("/a").plus("/b"));*/
    FOS_CHECK_RESULTS<Rec>(
        {*Obj::to_rec({{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}})},
        __(*Obj::to_rec({{"a", 1}})).plus(*Obj::to_rec({{"b", 2}})).plus(*Obj::to_rec({{"c", 3}, {"d", 4}})));
  }

  void test_mult() {
    // URI
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a")).mult(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a")).mult(u("/b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("b")));
    // FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b/")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("../b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("../b")));
    // FOS_CHECK_RESULTS<Rec>({Rec{{21, 10}, {48, 36}}}, __(Rec{{3, 2}, {6, 4}}).mult(Rec{{7, 5}, {8, 9}}));
  }

  void test_count() {
    FOS_CHECK_RESULTS<Uri>({3}, __({5, 4, 7}).plus(2).count());
    FOS_CHECK_RESULTS<Uri>({1}, __({5, 4, 7}).plus(2).count().count());
  }

  /* void test_where() { FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).where(_.is(_.eq(13)))); }*/

  void test_define_as_type() {
    Types::singleton()->saveType(id_p(FOS_TYPE_PREFIX "int/nat"), Obj::to_bcode()); //
    // Types::singleton()->writeToCache("/int/nat2", Obj::to_bcode()); //
    ////  FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).define(u("/int/nat"),
    ///_.block(_.is(_.gt(0)))).as(u("/int/nat")),
    //                      {{u("/int/nat"), _.is(_.gt(0))}});
    FOS_CHECK_RESULTS<Int>({1}, __(1).define(u(FOS_TYPE_PREFIX "int/nat"), _.block(_.is(_.gt(0)))), {{u(FOS_TYPE_PREFIX "int/nat"), _.is(_.gt(0))}});
    // FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).as(u("/int/nat")), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    // FOS_CHECK_RESULTS<Uri>({u("/int/nat")}, __(1).as(u("/int/nat")).type(), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    /*FOS_CHECK_RESULTS<Int>({Int(6, "/int/nat2")},
                           __(1)
                               .as(u("nat"))
                               .plus(Int(2, "nat"))
                               .to(u("x"))
                               .plus(Int(3, "nat"))
                               .type()
                               .from(_)
                               .define(u("/int/nat2"), _)
                               .from(u("x"))
                               .as(u("/int/nat2"))
                               .mult(Int(2, "/int/nat2")),
                           {{u("/int/nat"), _.is(_.gt(0))}, {u("/int/nat2"), _.is(_.gt(0))}, {u("x"), Int(3, "nat")}},
                           false);*/
  }


  FOS_RUN_TESTS( //
        FOS_RUN_TEST(test_to_from); //
        FOS_RUN_TEST(test_plus); //
        FOS_RUN_TEST(test_mult); //
        FOS_RUN_TEST(test_count); //
        FOS_RUN_TEST(test_relational_predicates); //
        // FOS_RUN_TEST(test_define_as_type); //
      )
} // namespace fhatos

SETUP_AND_LOOP();

#endif
