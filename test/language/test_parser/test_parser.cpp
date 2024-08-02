#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#define FOS_TEST_ON_BOOT

#include <language/parser.hpp>
#include <test_fhatos.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_no_input_parsing() { TEST_ASSERT_FALSE(Parser::singleton()->tryParseObj("").has_value()); }

  void test_start_inst_parsing() {
    FOS_CHECK_RESULTS<Rec>({"fhat"}, "'fh'.plus('at')");
    FOS_CHECK_RESULTS<Rec>({22}, "1.plus(21)");
    FOS_CHECK_RESULTS<Rec>({u("a/b")}, "a.plus(b)");
  }

  void test_noobj_parsing() {
    const Obj_p n = Parser::singleton()->tryParseObj("Ø").value();
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->o_type());
    TEST_ASSERT_TRUE(n->isNoObj());
    TEST_ASSERT_EQUAL_STRING("!b/noobj/!g[!!!g]!!", n->toString().c_str());
  }

  void test_bool_parsing() {
    Types::singleton()->saveType(id_p("/bool/fact"), Obj::to_bcode({}));
    const auto bools = List<Trip<string, bool, fURI>>(
        {{"true", true, *BOOL_FURI}, {"false", false, *BOOL_FURI}, {"fact[true]", true, BOOL_FURI->resolve("fact")}});
    for (const auto &trip: bools) {
      const Bool_p b = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::BOOL, b->o_type());
      TEST_ASSERT_EQUAL(get<1>(trip), b->bool_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *b->id());
    }
  }

  void test_int_parsing() {
    Types::singleton()->saveType(id_p("/int/zero"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/int/nat"), Obj::to_bcode({}));
    const auto ints = List<Trip<string, FL_INT_TYPE, fURI>>({{"45", 45, *INT_FURI},
                                                             {"0", 0, *INT_FURI},
                                                             {"-12", -12, *INT_FURI},
                                                             {"nat[100]", 100, INT_FURI->resolve("nat")},
                                                             {"zero[0]", 0, INT_FURI->resolve("zero")},
                                                             {"zero[2]", 2, INT_FURI->resolve("/int/zero")},
                                                             {"/int/zero[0]", 0, INT_FURI->resolve("zero")},
                                                             {"/int/zero[98]", 98, INT_FURI->resolve("/int/zero")},
                                                             {"/int/zero[001]", 1, INT_FURI->extend("zero")}});
    for (auto &trip: ints) {
      const ptr<Int> i = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::INT, i->o_type());
      TEST_ASSERT_EQUAL_INT(get<1>(trip), i->int_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *i->id());
    }
  }

  void test_real_parsing() {
    Types::singleton()->saveType(id_p("/real/zero"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/real/nat"), Obj::to_bcode({}));
    // REAL
    const auto reals =
        List<Trip<string, FL_REAL_TYPE, fURI>>({{"45.1", 45.1f, *REAL_FURI},
                                                {"0.0000", 0.0f, *REAL_FURI},
                                                {"-12.1", -12.1f, *REAL_FURI},
                                                {"nat[10.0]", 10.0f, REAL_FURI->resolve("nat")},
                                                {"zero[0.21]", 0.21f, REAL_FURI->resolve("zero")},
                                                {"zero[2.0]", 2.0f, REAL_FURI->resolve("/real/zero")},
                                                {"/real/zero[1.1]", 1.1f, REAL_FURI->resolve("zero")},
                                                {"/real/zero[98.00]", 98.00f, REAL_FURI->resolve("/real/zero")},
                                                {"/real/zero[001.1]", 1.1f, REAL_FURI->extend("zero")}});
    for (auto &trip: reals) {
      const Real_p r = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::REAL, r->o_type());
      TEST_ASSERT_EQUAL_INT(get<1>(trip), r->real_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *r->id());
    }
  }

  void test_uri_parsing() {
    Types::singleton()->saveType(id_p("/uri/x"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/uri/furi:"), Obj::to_bcode({}));
    const auto uris = List<Trip<string, fURI, fURI>>({{"<blah.com>", fURI("blah.com"), *URI_FURI},
                                                      {"ga", fURI("ga"), *URI_FURI},
                                                      {"x[x]", fURI("x"), URI_FURI->resolve("x")},
                                                      {"furi:[blah_com]", fURI("blah_com"), URI_FURI->resolve("furi:")},
                                                      {"/abc_2467", fURI("/abc_2467"), *URI_FURI}});
    for (auto &trip: uris) {
      const Uri_p u = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::URI, u->o_type());
      FOS_TEST_ASSERT_EQUAL_FURI(get<1>(trip), u->uri_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *u->id());
    }
  }

  void test_str_parsing() {
    Types::singleton()->saveType(id_p("/str/name"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/str/origin"), Obj::to_bcode({}));
    const auto strs = List<Trip<string, string, fURI>>(
        {{"'bob'", "bob", *STR_FURI},
         {"'ga'", "ga", *STR_FURI},
         {"name['fhat']", "fhat", STR_FURI->resolve("name")},
         {"'a long and winding\nroad of\nstuff'", "a long and winding\nroad of\nstuff", *STR_FURI},
         {"origin['abc_2467']", "abc_2467", STR_FURI->resolve("origin")}});
    for (auto &trip: strs) {
      const Str_p s = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::STR, s->o_type());
      TEST_ASSERT_EQUAL_STRING(get<1>(trip).c_str(), s->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *s->id());
    }
  }

  void test_lst_parsing() {
    // LST
    Types::singleton()->saveType(id_p("/int/nat"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/lst/atype"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/lst/btype"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/lst/ctype"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/bool/abool"), Obj::to_bcode({}));
    const auto lsts = List<Trip<string, List<Obj_p>, fURI>>(
        {{"['a',13,<actor>,false]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          *LST_FURI},
         {"['a' , 13,<actor> , false ]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          *LST_FURI},
         {"['a' ,    13 , <actor> ,    false  ]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          *LST_FURI},
         {"['a',    13 ,<actor>,   false]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          *LST_FURI},
         {"atype[['a',13 ,<actor>,   false]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("atype")},
         {"atype  [['a',13 ,<actor>,   false]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("atype")},
         {"btype[['a',  nat[13] , actor, abool   [false]]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("btype")},
         {"    btype[['a',  nat[13] ,actor,   abool [false]]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("btype")},
         {"ctype[['a',    13 , <actor>,  false]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("ctype")},
         {"   ctype    [['a',    13 ,<actor>,   false]]",
          {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
          LST_FURI->resolve("ctype")}});
    for (const auto &trip: lsts) {
      FOS_TEST_MESSAGE("!yTesting!! !blst!! form %s", std::get<0>(trip).c_str());
      const Lst_p l = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::LST, l->o_type());
      TEST_ASSERT_EQUAL_STRING("a", l->lst_get(share(Int(0)))->str_value().c_str());
      TEST_ASSERT_EQUAL_INT(13, l->lst_get(share(Int(1)))->int_value());
      FOS_TEST_ASSERT_EQUAL_FURI(ID("actor"), l->lst_get(share(Int(2)))->uri_value());
      TEST_ASSERT_FALSE(l->lst_get(share(Int(3)))->bool_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *l->id());
    }
  }

  void test_rec_parsing() {
    // REC
    Types::singleton()->saveType(id_p("/int/nat"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/rec/person"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/rec/atype"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/rec/btype"), Obj::to_bcode({}));
    Types::singleton()->saveType(id_p("/rec/ctype"), Obj::to_bcode({}));
    // Types::singleton()->saveType(id_p("/bool/abool"), Obj::to_bcode({}));
    List<string> recs = {"['a'=>13,actor=>false]", "['a' => 13,actor => false ]", "['a'=> 13 , actor=>false]",
                         "['a' =>    13 , actor =>    false  ]", "['a'=>    13 ,actor=>   false]",
                         "atype[['a'=>13 ,actor=>   false]]",
                         // "btype[['a'=>  nat[13] , actor=>   abool[false]]  ]",
                         "ctype[  ['a'=>    13 ,  actor=>   false]]"};
    for (const string &form: recs) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const Rec_p rc1 = Parser::singleton()->tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc1->o_type());
      TEST_ASSERT_EQUAL_INT(13, rc1->rec_get("a")->int_value()); // a's value is 13
      TEST_ASSERT_TRUE(rc1->rec_get(Obj::to_int(13))->isNoObj()); // no key is 13
      TEST_ASSERT_TRUE(rc1->rec_get(Obj::to_str("no key"))->isNoObj()); // no key is no key
      TEST_ASSERT_FALSE(rc1->rec_get(u_p("/rec/actor"))->bool_value());
    }

    recs = {"[:age=>nat[29],:name=>'dogturd']", "[:age=>nat[29],:name=>'dogturd']"
            /*"person?x[[age=>nat[29],name=>'dogturd']]", "person?x[[age=>nat[29],name=>'dogturd']]"*/};
    for (const string &form: recs) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! structure %s", form.c_str());
      const Rec_p rc2 = Parser::singleton()->tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc2->o_type());
      TEST_ASSERT_EQUAL_STRING("rec", rc2->id()->name());
      TEST_ASSERT_EQUAL_INT(29, rc2->rec_get(u(":age"))->int_value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc2->rec_get(u(":name"))->str_value().c_str());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->rec_get(13)->o_type()); // TODO
      TEST_ASSERT_TRUE(rc2->rec_get("no key")->isNoObj());
    }
    ///////////////////////////////////
    const ptr<Rec> rc2 = Parser::singleton()->tryParseObj("['a'=>13,/actor=>['b'=>1,'c'=>3]]").value();
    TEST_ASSERT_EQUAL(OType::REC, rc2->o_type());
    TEST_ASSERT_EQUAL_INT(13, rc2->rec_get("a")->int_value());
    //    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(ptr<Int>(new Int(13)))->otype());
    TEST_ASSERT_TRUE(rc2->rec_get("/actor")->isNoObj()); // it's a string, not a uri
    const ptr<Rec> rc3 = rc2->rec_get(u("/actor"));
    TEST_ASSERT_EQUAL(OType::REC, rc3->o_type());
    TEST_ASSERT_EQUAL_INT(1, rc3->rec_get("b")->int_value());
    TEST_ASSERT_EQUAL_INT(3, rc3->rec_get("c")->int_value());
    TEST_ASSERT_EQUAL_STRING("['a'=>13,/actor=>['b'=>1,'c'=>3]]",
                              Options::singleton()->printer<>()->strip(rc2->toString().c_str()));
  }

  void test_bytecode_parsing() {
    const ptr<BCode> bcode = FOS_PRINT_OBJ<BCode>(Parser::singleton()->tryParseObj("__().plus(mult(plus(3)))").value());
    TEST_ASSERT_EQUAL_INT(2, bcode->bcode_value().size());
    TEST_ASSERT_EQUAL_INT(1, bcode->bcode_value().at(1)->inst_arg(0)->bcode_value().size());
    ///
    FOS_CHECK_RESULTS<Rec>({"fhat"}, "__([1,2,'fhat']).get(2)");

    FOS_TEST_MESSAGE("Testing !yparenthese!! and !ybracket!! instructions...");
    //// TEST PAREN AND BRACKET INSTRUCTIONS
    FOS_CHECK_RESULTS({10}, "6.plus(4)");
    //  FOS_CHECK_RESULTS({10}, "6.plus[4]");
    //  FOS_CHECK_RESULTS({10}, "6.plus[1].plus[3]");
    // FOS_CHECK_RESULTS({10}, "6.plus[1].plus(3)");
  }

  void test_define_as_parsing() {
    FOS_CHECK_RESULTS<Int>({}, "define(/int/even,|mod(2).is(eq(0)))");
    FOS_CHECK_RESULTS<Uri>({u("/int/even")}, "__(32).as(even).type()");
    FOS_CHECK_RESULTS<Uri>({u("/int/even")}, "even[32].type()");
    FOS_CHECK_RESULTS<Uri>({u("/int/even")}, "__(even[32]).type()", {}, true);
    FOS_TEST_ERROR("even[1]");
    FOS_TEST_ERROR("even[3]");
    FOS_TEST_ERROR("even[5]");
    /////////
    Fluent(FOS_PRINT_OBJ<BCode>(
               Parser::singleton()->tryParseObj("define(/rec/person,|[name=>as(/str/),age=>is(gt(0))])").value()))
        .iterate();
    FOS_CHECK_RESULTS<Rec>({*Parser::singleton()->tryParseObj("person[[name=>'fhat',age=>29]]").value()},
                           "[name=>'fhat',age=>29].as(person)");
    FOS_TEST_ERROR("[name=>10,age=>23].as(person)");
    FOS_TEST_ERROR("[name=>'fhat',age=>-1].as(person)");
  }

  void test_to_from() {
    FOS_CHECK_RESULTS<Int>({10},
                           List<string>({"y.to(x)", "z.to(y)", "10.to(z)"})); // TODO: fix to() so it doesn't Ø on start
    FOS_CHECK_RESULTS<Int>({10}, "from(z)");
    FOS_CHECK_RESULTS<Int>({10}, "from(from(y))");
    FOS_CHECK_RESULTS<Int>({10}, "from(from(from(x)))");
    FOS_CHECK_RESULTS<Int>({10}, "*z");
    FOS_CHECK_RESULTS<Int>({10}, "**y");
    FOS_CHECK_RESULTS<Int>({10}, "***x", {}, true);
  }

  void test_process_thread_parsing() {
    for (const Pair<ID, Type_p> &pair: Exts::exts("/ext/process")) {
      Types::singleton()->saveType(id_p(pair.first), pair.second, true);
    }
    const ptr<BCode> bcode = FOS_PRINT_OBJ<BCode>(Parser::singleton()
                                                      ->tryParseObj("thread[[setup => |print('setup complete'),"
                                                                    "        loop  => |stop(/abc/)]].to(/abc/)")
                                                      .value());
    Fluent(bcode).iterate();
    Scheduler::singleton()->barrier(( Options::singleton()->router<Router>()->id()->toString() + "_wait").c_str(),
                                    [] { return Scheduler::singleton()->count("/abc/") == 0; });
  }

  void test_group_parsing() {
    FOS_CHECK_RESULTS<>(List<Obj>({*Obj::to_rec({{false, {1, 3}}, {true, {2, 4}}})}),
                        "__(0,1,2,3).plus(1).group(mod(2).eq(0))");
    ////////////////////
    FOS_CHECK_RESULTS<>(List<Obj>({*Obj::to_rec({{false, {2, 4}}, {true, {3, 5}}})}),
                        "__(0,1,2,3).plus(1).group(mod(2).eq(0)).by(plus(1))");
    FOS_CHECK_RESULTS<>(List<Obj>({*Obj::to_rec({{false, {2, 4}}, {true, {3, 5}}})}),
                        "__(0,1,2,3).plus(1).group().by(mod(2).eq(0)).by(plus(1))");
    FOS_TEST_ERROR("__(0,1,2,3).plus(1).group().by(mod(2).eq(0)).by(plus(1)).by(_).by(_)");
  }

  void test_window_parsing() { FOS_CHECK_RESULTS<>(List<Obj>{{1, 2}, {2, 3}, {3, 4}}, "[1,2,3,4].window([_,_])"); }


  FOS_RUN_TESTS( //
      for (Router *router //
           : List<Router *>{FOS_TEST_ROUTERS}) { //
        Options::singleton()->router<Router>(router); //
        router->clear();
        FOS_TEST_MESSAGE("!r!_Testing with %s!!\n", router->toString().c_str()); //
        FOS_RUN_TEST(test_no_input_parsing); //
        FOS_RUN_TEST(test_start_inst_parsing); //
        FOS_RUN_TEST(test_noobj_parsing); //
        FOS_RUN_TEST(test_bool_parsing); //
        FOS_RUN_TEST(test_int_parsing); //
        FOS_RUN_TEST(test_real_parsing); //
        FOS_RUN_TEST(test_uri_parsing); //
        FOS_RUN_TEST(test_str_parsing); //
        FOS_RUN_TEST(test_lst_parsing); //
        // FOS_RUN_TEST(test_rec_parsing); //
        FOS_RUN_TEST(test_bytecode_parsing); //
        ////////////// PARTICULAR MONOID IDIOMS
        FOS_RUN_TEST(test_define_as_parsing); //
        FOS_RUN_TEST(test_to_from); //
        FOS_RUN_TEST(test_process_thread_parsing); //
        FOS_RUN_TEST(test_group_parsing); //
        FOS_RUN_TEST(test_window_parsing); //


      })
}; // namespace fhatos

SETUP_AND_LOOP();


#endif
