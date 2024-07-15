#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#include <test_fhatos.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_no_input_parsing() { TEST_ASSERT_FALSE(Parser::singleton()->tryParseObj("").has_value()); }

  void test_start_inst_parsing() { FOS_CHECK_RESULTS<Rec>({"fhat"}, List<string>({"'fh'.plus('at')"})); }

  void test_noobj_parsing() {
    const Obj_p n = Parser::singleton()->tryParseObj("Ø").value();
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->o_type());
    TEST_ASSERT_TRUE(n->isNoObj());
    TEST_ASSERT_EQUAL_STRING("!bØ!!", n->toString().c_str());
  }

  void test_bool_parsing() {
    Types::singleton()->saveType("/bool/fact", Obj::to_bcode({}));
    for (const auto &trip: List<Trip<string, bool, fURI>>({{"true", true, *BOOL_FURI},
                                                           {"false", false, *BOOL_FURI},
                                                           {"fact[true]", true, BOOL_FURI->resolve("fact")}})) {
      const Bool_p b = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::BOOL, b->o_type());
      TEST_ASSERT_EQUAL(get<1>(trip), b->bool_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *b->id());
    }
  }

  void test_int_parsing() {
    Types::singleton()->saveType("/int/zero", Obj::to_bcode({}));
    Types::singleton()->saveType("/int/nat", Obj::to_bcode({}));
    for (auto &trip: List<Trip<string, FL_INT_TYPE, fURI>>({{"45", 45, *INT_FURI},
                                                            {"0", 0, *INT_FURI},
                                                            {"-12", -12, *INT_FURI},
                                                            {"nat[100]", 100, INT_FURI->resolve("nat")},
                                                            {"zero[0]", 0, INT_FURI->resolve("zero")},
                                                            {"zero[2]", 2, INT_FURI->resolve("/int/zero")},
                                                            {"/int/zero[0]", 0, INT_FURI->resolve("zero")},
                                                            {"/int/zero[98]", 98, INT_FURI->resolve("/int/zero")},
                                                            {"/int/zero[001]", 1, INT_FURI->extend("zero")}})) {
      const ptr<Int> i = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::INT, i->o_type());
      TEST_ASSERT_EQUAL_INT(get<1>(trip), i->int_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *i->id());
    }
  }

  void test_real_parsing() {
    Types::singleton()->saveType("/real/zero", Obj::to_bcode({}));
    Types::singleton()->saveType("/real/nat", Obj::to_bcode({}));
    // REAL
    for (auto &trip:
         List<Trip<string, FL_REAL_TYPE, fURI>>({{"45.1", 45.1f, *REAL_FURI},
                                                 {"0.0000", 0.0f, *REAL_FURI},
                                                 {"-12.1", -12.1f, *REAL_FURI},
                                                 {"nat[10.0]", 10.0f, REAL_FURI->resolve("nat")},
                                                 {"zero[0.21]", 0.21f, REAL_FURI->resolve("zero")},
                                                 {"zero[2.0]", 2.0f, REAL_FURI->resolve("/real/zero")},
                                                 {"/real/zero[1.1]", 1.1f, REAL_FURI->resolve("zero")},
                                                 {"/real/zero[98.00]", 98.00f, REAL_FURI->resolve("/real/zero")},
                                                 {"/real/zero[001.1]", 1.1f, REAL_FURI->extend("zero")}})) {
      const Real_p r = Parser::singleton()->tryParseObj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::REAL, r->o_type());
      TEST_ASSERT_EQUAL_INT(get<1>(trip), r->real_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *r->id());
    }
  }

  void test_uri_parsing() {
    Types::singleton()->saveType("/uri/x", Obj::to_bcode({}));
    Types::singleton()->saveType("/uri/furi:", Obj::to_bcode({}));
    for (auto &trip: List<Trip<string, fURI, fURI>>({{"<blah.com>", fURI("blah.com"), *URI_FURI},
                                                     {"ga", fURI("ga"), *URI_FURI},
                                                     {"x[x]", fURI("x"), URI_FURI->resolve("x")},
                                                     {"furi:[blah_com]", fURI("blah_com"), URI_FURI->resolve("furi:")},
                                                     {"/abc_2467", fURI("/abc_2467"), *URI_FURI}})) {
      const Uri_p u = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::URI, u->o_type());
      FOS_TEST_ASSERT_EQUAL_FURI(get<1>(trip), u->uri_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *u->id());
    }
  }

  void test_str_parsing() {
    Types::singleton()->saveType("/str/name", Obj::to_bcode({}));
    Types::singleton()->saveType("/str/origin", Obj::to_bcode({}));
    for (auto &trip: List<Trip<string, string, fURI>>(
             {{"'bob'", "bob", *STR_FURI},
              {"'ga'", "ga", *STR_FURI},
              {"name['fhat']", "fhat", STR_FURI->resolve("name")},
              {"'a long and winding\nroad of\nstuff'", "a long and winding\nroad of\nstuff", *STR_FURI},
              {"origin['abc_2467']", "abc_2467", STR_FURI->resolve("origin")}})) {
      const Str_p s = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::STR, s->o_type());
      TEST_ASSERT_EQUAL_STRING(get<1>(trip).c_str(), s->str_value().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *s->id());
    }
  }

  void test_lst_parsing() {
    // LST
    Types::singleton()->saveType("/lst/atype", Obj::to_bcode({}));
    Types::singleton()->saveType("/lst/btype", Obj::to_bcode({}));
    Types::singleton()->saveType("/lst/ctype", Obj::to_bcode({}));
    Types::singleton()->saveType("/bool/abool", Obj::to_bcode({}));
    for (auto &trip: List<Trip<string, List<Obj_p>, fURI>>(
             {{"['a',13,actor,false]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               *LST_FURI},
              {"['a' , 13,actor , false ]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               *LST_FURI},
              {"['a' ,    13 , actor ,    false  ]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               *LST_FURI},
              {"['a',    13 ,actor,   false]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               *LST_FURI},
              {"atype[['a',13 ,actor,   false]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("atype")},
              {"atype  [['a',13 ,actor,   false]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("atype")},
              {"btype[['a',  nat[13] , actor, abool   [false]]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("btype")},
              {"    btype[['a',  nat[13] ,actor,   abool [false]]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("btype")},
              {"ctype[['a',    13 , actor,  false]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("ctype")},
              {"   ctype    [['a',    13 ,actor,   false]]",
               {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
               LST_FURI->resolve("ctype")}})) {
      FOS_TEST_MESSAGE("!yTesting!! !blst!! form %s", std::get<0>(trip).c_str());
      const Lst_p l = Parser::singleton()->tryParseObj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::LST, l->o_type());
      TEST_ASSERT_EQUAL_STRING("a", l->lst_get(share(Int(0)))->str_value().c_str());
      TEST_ASSERT_EQUAL_INT(13, l->lst_get(share(Int(1)))->int_value());
      FOS_TEST_ASSERT_EQUAL_FURI(fURI("actor"), l->lst_get(share(Int(2)))->uri_value());
      TEST_ASSERT_FALSE(l->lst_get(share(Int(3)))->bool_value());
      FOS_TEST_ASSERT_EQUAL_FURI(get<2>(trip), *l->id());
    }
    ////////////
    FOS_CHECK_RESULTS<Rec>({"fhat"}, Fluent(Parser::singleton()->tryParseObj("__([1,2,'fhat']).get(2)").value()));
  }

  void test_rec_parsing() {
    // REC
    Types::singleton()->saveType("/rec/person", //
                                 Obj::to_bcode({})); //
    Types::singleton()->saveType("/rec/atype", Obj::to_bcode({}));
    Types::singleton()->saveType("/rec/btype", Obj::to_bcode({}));
    Types::singleton()->saveType("/rec/ctype", Obj::to_bcode({}));
    Types::singleton()->saveType("/bool/abool", Obj::to_bcode({}));
    List<string> forms = {"['a'=>13,actor=>false]",
                          "['a' => 13,actor => false ]",
                          "['a'=> 13 , actor=>false]",
                          "['a' =>    13 , actor =>    false  ]",
                          "['a'=>    13 ,actor=>   false]",
                          "atype[['a'=>13 ,actor=>   false]]",
                          "btype[['a'=>  nat[13] , actor=>   abool[false]]  ]",
                          "ctype[  ['a'=>    13 ,  actor=>   false]]"};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const ptr<Rec> rc1 = Parser::singleton()->tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc1->o_type());
      TEST_ASSERT_EQUAL_INT(13, rc1->rec_get("a")->int_value());
      TEST_ASSERT_TRUE(rc1->rec_get(ptr<Int>(new Int(13)))->isNoObj());
      TEST_ASSERT_TRUE(rc1->rec_get(ptr<Str>(new Str("no key")))->isNoObj());
      TEST_ASSERT_FALSE(rc1->rec_get(share(u("actor")))->bool_value());
    }

    forms = {"person[[age=>nat[29],name=>'dogturd']]", "person[[age=>nat[29],name=>'dogturd']]"
             /*"person?x[[age=>nat[29],name=>'dogturd']]", "person?x[[age=>nat[29],name=>'dogturd']]"*/};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! structure %s", form.c_str());
      const Rec_p rc2 = Parser::singleton()->tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc2->o_type());
      TEST_ASSERT_EQUAL_STRING("person", rc2->id()->lastSegment().c_str());
      TEST_ASSERT_EQUAL_INT(29, rc2->rec_get(u("age"))->int_value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc2->rec_get(u("name"))->str_value().c_str());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->rec_get(13)->o_type()); // TODO
      TEST_ASSERT_TRUE(rc2->rec_get("no key")->isNoObj());
      // TEST_ASSERT_TRUE(rc2->rec_get(Obj::to_uri("actor@127.0.0.1"))->isNoObj());
    }
    ///////////////////////////////////
    const ptr<Rec> rc2 = Parser::singleton()->tryParseObj("['a'=>13,actor=>['b'=>1,'c'=>3]]").value();
    TEST_ASSERT_EQUAL(OType::REC, rc2->o_type());
    TEST_ASSERT_EQUAL_INT(13, rc2->rec_get("a")->int_value());
    //    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(ptr<Int>(new Int(13)))->otype());
    TEST_ASSERT_TRUE(rc2->rec_get("actor")->isNoObj()); // it's a string, not a uri
    const ptr<Rec> rc3 = rc2->rec_get(u("actor"));
    TEST_ASSERT_EQUAL(OType::REC, rc3->o_type());
    TEST_ASSERT_EQUAL_INT(1, rc3->rec_get("b")->int_value());
    TEST_ASSERT_EQUAL_INT(3, rc3->rec_get("c")->int_value());
    TEST_ASSERT_EQUAL_STRING("['a'=>13,actor=>['b'=>1,'c'=>3]]",
                             GLOBAL_OPTIONS->printer<>()->strip(rc2->toString().c_str()));

    ////// match testing
    Fluent(FOS_PRINT_OBJ<BCode>(
               Parser::singleton()->tryParseObj("define(/rec/person,[name=>as(/str/),age=>is(gt(0))])").value()))
        .iterate();
    FOS_CHECK_RESULTS<Rec>({*Parser::singleton()->tryParseObj("person[[name=>'fhat',age=>29]]").value()},
                           Fluent(Parser::singleton()->tryParseObj("__([name=>'fhat',age=>29]).as(person)").value()),
                           {}, false);
    FOS_TEST_ERROR("__([name=>10,age=>23]).as(person)");
    FOS_TEST_ERROR("__([name=>'fhat',age=>-1]).as(person)");
  }

  void test_nested_bytecode_parsing() {
    const ptr<BCode> bcode = FOS_PRINT_OBJ<BCode>(Parser::singleton()->tryParseObj("__().plus(mult(plus(3)))").value());
    TEST_ASSERT_EQUAL_INT(2, bcode->bcode_value().size());
    TEST_ASSERT_EQUAL_INT(1, bcode->bcode_value().at(1)->inst_arg(0)->bcode_value().size());
    //   TEST_ASSERT_EQUAL_INT(
    // 1, bcode->v_insts()->at(0)->arg(0)->as<Bytecode>()->v_insts().at(0)->arg(0)->as<Bytecode>()->v_insts().size());
  }

  void test_define_as_parsing() {
    GLOBAL_OPTIONS->router<Router>()->clear();
    FOS_CHECK_RESULTS<Int>({}, Fluent(Parser::singleton()->tryParseObj("define(/int/even,mod(2).is(eq(0)))").value()),
                           {}, false);
    FOS_CHECK_RESULTS<Uri>({u("/int/even")}, Fluent(Parser::singleton()->tryParseObj("__(32).as(even).type()").value()),
                           {}, false);
    FOS_CHECK_RESULTS<Uri>({Uri(fURI("/int/even"))},
                           Fluent(Parser::singleton()->tryParseObj("__(even[32]).type()").value()), {}, true);
  }

  void test_to_from() {
    if (GLOBAL_OPTIONS->router<Router>()->toString() != "MqttRouter") {
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"__(y).to(x)", "__(z).to(y)", "__(10).to(z)"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"from(z)"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"from(from(y))"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"from(from(from(x)))"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"*z"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"**y"}), {}, false);
      FOS_CHECK_RESULTS<Int>({10}, List<string>({"***x"}), {});
    }
  }

  void test_bcode_parsing() {
    Types::singleton()->loadExt("/ext/process");
    const ptr<BCode> bcode = FOS_PRINT_OBJ<BCode>(Parser::singleton()
                                                      ->tryParseObj("thread[[setup => print('setup complete'),"
                                                                    "        loop  => to(/abc/)]].to(/abc/)")
                                                      .value());
    Fluent(bcode).iterate();
    Scheduler::singleton()->barrier((Router::current()->id()->toString() + "_wait").c_str(),
                                    [] { return Scheduler::singleton()->count("/abc/") == 0; });
  }


  FOS_RUN_TESTS( //
      for (Router *router //
           : List<Router *>{FOS_TEST_ROUTERS}) { //
        GLOBAL_OPTIONS->ROUTING = router; //
        router->clear();
        LOG(INFO, "!r!_Testing with %s!!\n", router->toString().c_str()); //
        // FOS_RUN_TEST(test_basic_parser); //
        FOS_RUN_TEST(test_no_input_parsing); //
        FOS_RUN_TEST(test_start_inst_parsing); //
        FOS_RUN_TEST(test_noobj_parsing); //
        FOS_RUN_TEST(test_bool_parsing); //
        FOS_RUN_TEST(test_int_parsing); //
        FOS_RUN_TEST(test_real_parsing); //
        FOS_RUN_TEST(test_uri_parsing); //
        FOS_RUN_TEST(test_str_parsing); //
        FOS_RUN_TEST(test_lst_parsing); //
        FOS_RUN_TEST(test_rec_parsing); //
        FOS_RUN_TEST(test_nested_bytecode_parsing); //
        FOS_RUN_TEST(test_define_as_parsing); //
        FOS_RUN_TEST(test_to_from); //
        FOS_RUN_TEST(test_bcode_parsing); //
      })
}; // namespace fhatos

SETUP_AND_LOOP();


#endif
