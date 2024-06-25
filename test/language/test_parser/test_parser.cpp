#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#include <test_fhatos.hpp>
//
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <language/parser.hpp>
#include <language/processor.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_basic_parser() {
    try {
      //  parser.parseToFluent("__(actor@127.9.9.1).<=(__.plus(temp),'hello').plus(dmc).=>(__,__)")
      //     ->forEach<Uri>([](const ptr<Uri> uri) { LOG(INFO, "==>%s\n", uri->value().toString().c_str()); });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int>(Parser::tryParseObj("__(15).plus(__)").value()).forEach([](const ptr<Int> &i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int> *p = new Processor<Int>(Parser::tryParseObj("__(15).plus(1).plus(__.plus(5))").value());
      p->forEach([](const ptr<Int> &i) { LOG(INFO, "==>%s\n", i->toString().c_str()); });
    } catch (fError error) {
      LOG_EXCEPTION(error);
    }
  }

  void test_no_input_parsing() { TEST_ASSERT_FALSE(Parser::tryParseObj("").has_value()); }

  void test_start_inst_parsing() {
    // FOS_CHECK_ARGS<Obj>({15}, parser->parse("__(15)")->startInst());
    // FOS_CHECK_ARGS<Obj>({15, -10, 0}, parser->parse("__(15,-10,0)")->startInst());
    //  FOS_CHECK_ARGS<Obj>({"fhat", "os", 69}, parser->parse("__('fhat','os',69)")->startInst());
    //  FOS_CHECK_ARGS<Obj>({new Rec({{new Str("a"), new Int(2)}})}, parser->parse("__(['a'=>2])")->startInst());
    //  FOS_CHECK_ARGS<Obj>({new Rec({{new Str("a"), new Int(2)}, {new Str("b"), new Int(3)}})},
    //                    parser->parse("__(['a'=>2,'b'=>3])")->startInst());
    /*   FOS_CHECK_ARGS<Obj>(
           {new Rec({{ share(Str("a")),  share(Int(2))}, { share(Str("b")), ptr<Rec>(new Rec({{share(Int(3)),
       share(Uri("http://fhatos.org"))}}))}})},
           parser->parse("__(['a'=>2,'b'=>[3=>http://fhatos.org]])")->startInst());*/
  }

  void test_noobj_parsing() {
    // const Obj_p n = ObjParser::tryParseObj("Ø").value();
    // TEST_ASSERT_EQUAL(OType::NOOBJ, n->o_range());
    // TEST_ASSERT_EQUAL_STRING("!bØ!!", n->toString().c_str());
  }

  void test_bool_parsing() {
    for (const auto &pair: List<Pair<string, bool>>({{"true", true}, {"false", false}})) {
      const ptr<Bool> b = Parser::tryParseObj(pair.first).value();
      TEST_ASSERT_EQUAL(OType::BOOL, b->o_range());
      TEST_ASSERT_EQUAL_INT(pair.second, b->bool_value());
    }
  }

  void test_int_parsing() {
    for (auto &pair: List<Pair<string, int>>({{"45", 45}, {"0", 0}, {"-12", -12}})) {
      const ptr<Int> i = Parser::tryParseObj(pair.first).value();
      TEST_ASSERT_EQUAL(OType::INT, i->o_range());
      TEST_ASSERT_EQUAL_INT(pair.second, i->int_value());
      // delete i;
    }
    // TYPED INT
    for (const auto &pair: List<Pair<string, Int>>({{"nat[100]", Int(100, "/int/nat")},
                                                    {"zero[0]", Int(0, "/int/zero")},
                                                    {"z[-100]", Int(-100, "/int/z")}})) {
      const ptr<Int> i = Parser::tryParseObj(pair.first).value();
      TEST_ASSERT_EQUAL(OType::INT, i->o_range());
      TEST_ASSERT_EQUAL_STRING(pair.second.id()->toString().c_str(), i->id()->toString().c_str());
      TEST_ASSERT_EQUAL_INT(pair.second.int_value(), i->int_value());
    }
    // ID/TYPE INT
    /* const ptr<Int> i = FOS_PRINT_OBJ(Parser::parseObj<Int>("x@/nat[25]"));
     TEST_ASSERT_EQUAL(OType::INT, i->o_range());
     // TODO:    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x@/int/nat"), *i->id());
     // FOS_TEST_ASSERT_EQUAL_FURI(fURI("x"), i->id());
     TEST_ASSERT_EQUAL_INT(25, i->int_value());*/
    // TODO: TEST_ASSERT_EQUAL_STRING("x@/nat[25]", FOS_DEFAULT_PRINTER::singleton()->strip(i->toString().c_str()));
  }

  void test_real_parsing() {
    // REAL
    for (auto pair: List<Pair<string, float>>({{"45.54", 45.54}, {"0.0", 0.0}, {"-12.534678", -12.534678}})) {
      const ptr<Real> r = Parser::tryParseObj(pair.first).value();
      TEST_ASSERT_EQUAL(OType::REAL, r->o_range());
      TEST_ASSERT_FLOAT_WITHIN(0.01, pair.second, r->real_value());
    }
  }

  void test_uri_parsing() {
    for (auto pair: List<Pair<string, fURI>>({{"blah.com", fURI("blah.com")},
                                              {"ga", fURI("ga")},
                                              {"x", fURI("x")},
                                              {"blah.com", fURI("blah.com")},
                                              {"/abc_2467", fURI("/abc_2467")}})) {
      const ptr<Uri> u = Parser::tryParseObj(pair.first).value();
      TEST_ASSERT_EQUAL(OType::URI, u->o_range());
      TEST_ASSERT_EQUAL_STRING(pair.second.toString().c_str(), u->uri_value().toString().c_str());
    }
  }

  void test_str_parsing() {
    const ptr<Str> s = Parser::tryParseObj("'fhatty-the-pig'").value();
    TEST_ASSERT_EQUAL(OType::STR, s->o_range());
    TEST_ASSERT_EQUAL_STRING("fhatty-the-pig", s->str_value().c_str());
  }

  void test_rec_parsing() {
    // REC
    List<string> forms = {"['a'=>13,actor@127.0.0.1=>false]",
                          "['a' => 13,actor@127.0.0.1 => false ]",
                          "['a'=> 13 , actor@127.0.0.1=>false]",
                          "['a' =>    13 , actor@127.0.0.1 =>    false  ]",
                          "['a'=>    13 ,actor@127.0.0.1=>   false]",
                          "atype[['a'=>13 ,actor@127.0.0.1=>   false]]",
                          "btype[['a'=>  nat[13] , actor@127.0.0.1=>   abool[false]]]",
                          "ctype[['a'=>    13 ,  actor@127.0.0.1=>   false]]"};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const ptr<Rec> rc1 = Parser::tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc1->o_range());
      TEST_ASSERT_EQUAL_INT(13, rc1->rec_get("a")->int_value());
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(ptr<Int>(new Int(13)))->otype());
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(ptr<Str>(new Str("no key")))->otype());
      // TEST_ASSERT_FALSE(rc1->get<Bool>(ptr<Uri>(new Uri("actor@127.0.0.1")))->value());
    }

    forms = {"person[[age=>nat[29],name=>'dogturd']]", "person[[age=>nat[29],name=>'dogturd']]",
             "person?x[[age=>nat[29],name=>'dogturd']]", "person?x[[age=>nat[29],name=>'dogturd']]"};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! structure %s", form.c_str());
      const Rec_p rc2 = Parser::tryParseObj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc2->o_range());
      // TEST_ASSERT_EQUAL_STRING("person", rc2->id()->lastSegment().c_str());
      TEST_ASSERT_EQUAL_INT(29, rc2->rec_get(u("age"))->int_value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc2->rec_get(u("name"))->str_value().c_str());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->rec_get(13)->o_range()); // TODO
      TEST_ASSERT_TRUE(rc2->rec_get("no key")->isNoObj());
      // TEST_ASSERT_TRUE(rc2->rec_get(Obj::to_uri("actor@127.0.0.1"))->isNoObj());
    }
    ///////////////////////////////////
    const ptr<Rec> rc2 = Parser::tryParseObj("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]").value();
    TEST_ASSERT_EQUAL(OType::REC, rc2->o_range());
    TEST_ASSERT_EQUAL_INT(13, rc2->rec_get("a")->int_value());
    //    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(ptr<Int>(new Int(13)))->otype());
    TEST_ASSERT_TRUE(rc2->rec_get("actor@127.0.0.1")->isNoObj()); // it's a string, not a uri
    const ptr<Rec> rc3 = rc2->rec_get(u("actor@127.0.0.1"));
    TEST_ASSERT_EQUAL(OType::REC, rc3->o_range());
    TEST_ASSERT_EQUAL_INT(1, rc3->rec_get("b")->int_value());
    TEST_ASSERT_EQUAL_INT(3, rc3->rec_get("c")->int_value());
    TEST_ASSERT_EQUAL_STRING("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]",
                             FOS_DEFAULT_PRINTER::singleton()->strip(rc2->toString().c_str()));
  }

  void test_bcode_parsing() {
    Scheduler<FOS_DEFAULT_ROUTER>::singleton();
    const ptr<BCode> bcode =
        FOS_PRINT_OBJ<BCode>(Parser::tryParseObj("<=(scheduler@kernel?spawn,thread["
                                                    "[id    => example,"
                                                    " setup => __(0).ref(x).print('setup complete'),"
                                                    " loop  => <=(scheduler@kernel?destroy,example)]])")
                                 .value());
    auto process = Processor<Str>(bcode);
    process.forEach([](const ptr<Str> s) { LOG(INFO, "RESULT: %s", s->str_value().c_str()); });
    Scheduler<>::singleton()->barrier("wait");
  }

  void test_nested_bytecode_parsing() {
    const ptr<BCode> bcode = FOS_PRINT_OBJ<BCode>(Parser::tryParseObj("__().plus(mult(plus(3)))").value());
    TEST_ASSERT_EQUAL_INT(2, bcode->bcode_value().size());
    TEST_ASSERT_EQUAL_INT(1, bcode->bcode_value().at(1)->inst_arg(0)->bcode_value().size());
    //   TEST_ASSERT_EQUAL_INT(
    // 1, bcode->v_insts()->at(0)->arg(0)->as<Bytecode>()->v_insts().at(0)->arg(0)->as<Bytecode>()->v_insts().size());
  }

  void test_define_as_parsing() {
    FOS_CHECK_RESULTS<Int>({0}, Fluent(Parser::tryParseObj("__(0).define(even,__().mod(2).is(eq(0))").value()), {},
                           false);
    FOS_CHECK_RESULTS<Int>({Int(32, "even")}, Fluent(Parser::tryParseObj("__(32).as(even)").value()), {}, false);
    // FOS_CHECK_RESULTS<Uri>({Uri(fURI("even"))}, Fluent(parser->parse("__(even[32]).as()")), {}, true);
  }

  FOS_RUN_TESTS( //
                 // FOS_RUN_TEST(test_basic_parser); //
      FOS_RUN_TEST(test_no_input_parsing); //
      FOS_RUN_TEST(test_start_inst_parsing); //
      FOS_RUN_TEST(test_noobj_parsing); //
      FOS_RUN_TEST(test_bool_parsing); //
      FOS_RUN_TEST(test_int_parsing); //
      FOS_RUN_TEST(test_real_parsing); //
      FOS_RUN_TEST(test_uri_parsing); //
      FOS_RUN_TEST(test_str_parsing); //
      FOS_RUN_TEST(test_rec_parsing); //
      FOS_RUN_TEST(test_nested_bytecode_parsing); //
      // FOS_RUN_TEST(test_bcode_parsing); //
      FOS_RUN_TEST(test_define_as_parsing); //
  )
}; // namespace fhatos

SETUP_AND_LOOP();


#endif
