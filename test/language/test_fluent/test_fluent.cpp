#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

// #include <atomic>
#include <language/fluent.hpp>
#include <language/instructions.hpp>
#include <process/router/local_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
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

    __(List<Obj>{32, 45}).plus(10).plus(15).forEach<Int>([](const Int_p e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });

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

  void test_to_from() {
    GLOBAL_OPTIONS->router<Router>()->clear();
    FOS_CHECK_RESULTS<Int>({2}, __(1).to(u("a")).plus(_.from(u("a"))), {{u("a"), 1}});
    FOS_CHECK_RESULTS<Int>({23}, __(10).to(u("a")).plus(3).plus(_.from(u("a"))), {{u("a"), 10}});
    FOS_CHECK_RESULTS<Str>({"fhatos"}, __("fhat").to(u("a")).plus("os"), {{u("a"), "fhat"}});
    /*FOS_CHECK_RESULTS<Str>(
        {"fhaty", "pigy"},
        __(List<Obj>{"fhat", "pig"})
            .bswitch({{*_.is(_.gt("gonzo")).bcode, *_.to(u("b")).bcode}, {*_.bcode, *_.to(u("c")).bcode}})
            .plus("y"),
        {{u("b"), "pig"}, {u("c"), "fhat"}});*/
    GLOBAL_OPTIONS->router<Router>()->clear();
  }

  void test_relational_predicates() {
    //// INT
    FOS_CHECK_RESULTS<Int>({1}, __(1).is(_.eq(1)));
    FOS_CHECK_RESULTS<Int>({}, __(1).is(_.neq(1)));
    FOS_CHECK_RESULTS<Int>({12}, __(List<Obj>{1, 2, 3}).plus(10).is(_.eq(12)));
    FOS_CHECK_RESULTS<Int>({11, 13}, __(List<Obj>{1, 2, 3}).plus(10).is(_.neq(12)));
    FOS_CHECK_RESULTS<Int>({13}, __(List<Obj>{1, 2, 3}).plus(10).is(_.gt(12)));
    FOS_CHECK_RESULTS<Int>({12, 13}, __(List<Obj>{1, 2, 3}).plus(10).is(_.gte(12)));
    FOS_CHECK_RESULTS<Int>({11}, __(List<Obj>{1, 2, 3}).plus(10).is(_.lt(12)));
    FOS_CHECK_RESULTS<Int>({11, 12}, __(List<Obj>{1, 2, 3}).plus(10).is(_.lte(12)));
    //// REAL
    FOS_CHECK_RESULTS<Int>({1.0f}, __(1.0f).is(_.eq(1.0f)));
    FOS_CHECK_RESULTS<Int>({}, __(1.0f).is(_.neq(1.0f)));
    FOS_CHECK_RESULTS<Int>({12.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.eq(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f, 13.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.neq(12.0f)));
    FOS_CHECK_RESULTS<Int>({13.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gt(12.0f)));
    FOS_CHECK_RESULTS<Int>({12.0f, 13.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gte(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lt(12.0f)));
    FOS_CHECK_RESULTS<Int>({11.0f, 12.0f}, __(List<Obj>{1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lte(12.0f)));
    //// STR
    FOS_CHECK_RESULTS<Int>({"1"}, __("1").is(_.eq("1")));
    FOS_CHECK_RESULTS<Int>({}, __("1").is(_.neq("1")));
    FOS_CHECK_RESULTS<Int>({"20"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.eq("20")));
    FOS_CHECK_RESULTS<Int>({"10", "30"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.neq("20")));
    FOS_CHECK_RESULTS<Int>({"30"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.gt("20")));
    FOS_CHECK_RESULTS<Int>({"20", "30"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.gte("20")));
    FOS_CHECK_RESULTS<Int>({"10"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.lt("20")));
    FOS_CHECK_RESULTS<Int>({"10", "20"}, __(List<Obj>{"1", "2", "3"}).plus("0").is(_.lte("20")));
  }

  void test_plus() {
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(false));
    FOS_CHECK_RESULTS<Bool>({true}, __(true).plus(true));
    FOS_CHECK_RESULTS<Bool>({false, true, false}, __(List<Obj>{false, true, false}).plus(false));
    //
    FOS_CHECK_RESULTS<Int>({3}, __(1).plus(2));
    FOS_CHECK_RESULTS<Int>({54, 50, 46}, __(List<Obj>{1, 2, 3}).plus(10).plus(_).plus(_.plus(2)));
    //
    FOS_CHECK_RESULTS<Real>({46.5f}, __(1.121f).plus(10.002f).plus(_).plus(_.plus(2.0f)));
    FOS_CHECK_RESULTS<Real>({54.4f, 50.4f, 46.4f},
                            __(List<Obj>{1.05f, 2.05f, 3.05f}).plus(10.05f).plus(_).plus(_.plus(2.0f)));
    //
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).plus(u("/a")).plus(u("/b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//b")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).plus(u("/a/")).plus(u("b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//../b/")},
                           __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b/")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a//../b/.")},
                           __(u("http://fhatos.org")).plus(u("/a/")).plus(u("../b")).plus(u(".")));
    //
    FOS_CHECK_RESULTS<Str>({"http://fhatos.org/a/b", "fhat.pig/a/b"},
                           __(List<Obj>{"http://fhatos.org", "fhat.pig"}).plus("/a").plus("/b"));
    FOS_CHECK_RESULTS<Rec>(
        {*Obj::to_rec({{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}})},
        __(*Obj::to_rec({{"a", 1}})).plus(*Obj::to_rec({{"b", 2}})).plus(*Obj::to_rec({{"c", 3}, {"d", 4}})));
  }

  void test_mult() {
    // URI
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a")).mult(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a")).mult(u("/b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("b")));
    FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("b/")));
    // FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b/")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("../b/")));
    // FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/b")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("../b")));
    // FOS_CHECK_RESULTS<Rec>({Rec{{21, 10}, {48, 36}}}, __(Rec{{3, 2}, {6, 4}}).mult(Rec{{7, 5}, {8, 9}}));
  }

  void test_count() {
    FOS_CHECK_RESULTS<Uri>({3}, __(List<Obj>{5, 4, 7}).plus(2).count());
    FOS_CHECK_RESULTS<Uri>({1}, __(List<Obj>{5, 4, 7}).plus(2).count().count());
  }

  /* void test_where() { FOS_CHECK_RESULTS<Int>({13}, __({1, 2, 3}).plus(10).where(_.is(_.eq(13)))); }*/

  void test_define_as_type() {
    FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).define(u("/int/nat"), _.is(_.gt(0))).as(u("/int/nat")),
                           {{u("/int/nat"), _.is(_.gt(0))}});
    FOS_CHECK_RESULTS<Int>({1}, __(1).define(u("/int/nat"), _.is(_.gt(0))), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    FOS_CHECK_RESULTS<Int>({Int(1, "/int/nat")}, __(1).as(u("/int/nat")), {{u("/int/nat"), _.is(_.gt(0))}}, false);
    FOS_CHECK_RESULTS<Uri>({u("/int/nat")}, __(1).as(u("/int/nat")).type(), {{u("/int/nat"), _.is(_.gt(0))}}, false);
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
      Types::singleton(); //
      for (fhatos::Router * router //
           : List<Router *>{fhatos::LocalRouter::singleton(), //
                            fhatos::MqttRouter::singleton()}) { //
        GLOBAL_OPTIONS->ROUTING = router; //
        LOG(INFO, "!r!_Testing with %s!!\n", router->toString().c_str()); //
        Types::singleton()->writeToCache("/int/nat", Obj::to_bcode({})); //
        Types::singleton()->writeToCache("/int/nat2", Obj::to_bcode({})); //
        FOS_RUN_TEST(test_to_from); //
        FOS_RUN_TEST(test_plus); //
        FOS_RUN_TEST(test_mult); //
        FOS_RUN_TEST(test_count); //
        FOS_RUN_TEST(test_relational_predicates); //
        FOS_RUN_TEST(test_define_as_type); //
      })
} // namespace fhatos

SETUP_AND_LOOP();

#endif
