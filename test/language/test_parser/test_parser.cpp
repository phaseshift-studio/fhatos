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
      Parser parser;
      //  parser.parseToFluent("__(actor@127.9.9.1).<=(__.plus(temp),'hello').plus(dmc).=>(__,__)")
      //     ->forEach<Uri>([](const ptr<Uri> uri) { LOG(INFO, "==>%s\n", uri->value().toString().c_str()); });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int>(parser.parse("__(15).plus(__)")).forEach([](const ptr<Int> &i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int> *p = new Processor<Int>(parser.parse("__(15).plus(1).plus(__.plus(5))"));
      p->forEach([](const ptr<Int> &i) { LOG(INFO, "==>%s\n", i->toString().c_str()); });
    } catch (fError error) {
      LOG_EXCEPTION(error);
    }
  }

  void test_no_input_parsing() {
    Parser *parser = new Parser();
    ptr<Bytecode> bcode = parser->parse("");
    TEST_ASSERT_EQUAL_INT(0, bcode->v_insts()->size());
    delete parser;
  }

  void test_start_inst_parsing() {
    Parser *parser = new Parser();
    FOS_CHECK_ARGS<Obj>({15}, parser->parse("__(15)")->startInst());
    FOS_CHECK_ARGS<Obj>({15, -10, 0}, parser->parse("__(15,-10,0)")->startInst());
    // FOS_CHECK_ARGS<Obj>({"fhat", "os", 69}, parser->parse("__('fhat','os',69)")->startInst());
    // FOS_CHECK_ARGS<Obj>({new Rec({{new Str("a"), new Int(2)}})}, parser->parse("__(['a'=>2])")->startInst());
    // FOS_CHECK_ARGS<Obj>({new Rec({{new Str("a"), new Int(2)}, {new Str("b"), new Int(3)}})},
    //                   parser->parse("__(['a'=>2,'b'=>3])")->startInst());
    /*   FOS_CHECK_ARGS<Obj>(
           {new Rec({{ share(Str("a")),  share(Int(2))}, { share(Str("b")), ptr<Rec>(new Rec({{share(Int(3)),
       share(Uri("http://fhatos.org"))}}))}})},
           parser->parse("__(['a'=>2,'b'=>[3=>http://fhatos.org]])")->startInst());*/
    delete parser;
  }

  void test_noobj_parsing() {
    const ptr<NoObj> n = Parser::parseObj<NoObj>(string("Ø"));
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->otype());
    TEST_ASSERT_EQUAL_STRING("Ø", n->toString().c_str());
  }

  void test_bool_parsing() {
    for (auto pair: List<Pair<string, bool>>({{"true", true}, {"false", false}})) {
      const ptr<Bool> b = Parser::parseObj<Bool>(pair.first);
      TEST_ASSERT_EQUAL(OType::BOOL, b->otype());
      TEST_ASSERT_EQUAL_INT(pair.second, b->value());
    }
  }

  void test_int_parsing() {
    for (auto pair: List<Pair<string, int>>({{"45", 45}, {"0", 0}, {"-12", -12}})) {
      const ptr<Int> i = Parser::parseObj<Int>(pair.first);
      TEST_ASSERT_EQUAL(OType::INT, i->otype());
      TEST_ASSERT_EQUAL_INT(pair.second, i->value());
      // delete i;
    }
    // TYPED INT
    for (const auto &pair: List<Pair<string, Int>>({{"nat[100]", Int(100, Int::_t("nat"))},
                                                    {"zero[0]", Int(0, Int::_t("zero"))},
                                                    {"z[-100]", Int(-100, Int::_t("z"))}})) {
      const ptr<Int> i = Parser::parseObj<Int>(pair.first);
      TEST_ASSERT_EQUAL(OType::INT, i->otype());
      // TEST_ASSERT_EQUAL_STRING(pair.second.type()->toString().c_str(), i->type()->toString().c_str());
      TEST_ASSERT_EQUAL_INT(pair.second.value(), i->value());
    }
    // ID/TYPE INT
    const ptr<Int> i = FOS_PRINT_OBJ(Parser::parseObj<Int>("x@/nat[25]"));
    TEST_ASSERT_EQUAL(OType::INT, i->otype());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x@/int/nat"), *i->type()->v_furi());
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("x"), i->id());
    TEST_ASSERT_EQUAL_INT(25, i->value());
    TEST_ASSERT_EQUAL_STRING("x@/nat[25]", FOS_DEFAULT_PRINTER::singleton()->strip(i->toString().c_str()));
  }

  void test_real_parsing() {
    // REAL
    for (auto pair: List<Pair<string, float>>({{"45.54", 45.54}, {"0.0", 0.0}, {"-12.534678", -12.534678}})) {
      const ptr<Real> r = Parser::parseObj<Real>(pair.first);
      TEST_ASSERT_EQUAL(OType::REAL, r->otype());
      TEST_ASSERT_FLOAT_WITHIN(0.01, pair.second, r->value());
    }
  }

  void test_uri_parsing() {
    for (auto pair: List<Pair<string, Uri>>({{"blah.com", Uri("blah.com")},
                                             {"ga", Uri("ga")},
                                             {"x", Uri("x")},
                                             {"blah.com", Uri("blah.com")},
                                             {"/abc_2467", Uri("/abc_2467")}})) {
      const ptr<Uri> u = Parser::parseObj<Uri>(pair.first);
      TEST_ASSERT_EQUAL(OType::URI, u->otype());
      TEST_ASSERT_EQUAL_STRING(pair.second.toString().c_str(), u->value().toString().c_str());
    }
  }

  void test_str_parsing() {
    const ptr<Str> s = Parser::parseObj<Str>(string("'fhatty-the-pig'"));
    TEST_ASSERT_EQUAL(OType::STR, s->otype());
    TEST_ASSERT_EQUAL_STRING("fhatty-the-pig", s->value().c_str());
  }

  void test_rec_parsing() {
    // REC
    List<string> forms = {"['a'=>13,actor@127.0.0.1=>false]",
                          "['a' => 13,actor@127.0.0.1 => false ]",
                          "['a'=> 13 , actor@127.0.0.1=>false]",
                          "['a' =>    13 , actor@127.0.0.1 =>    false  ]",
                          "['a'=>    13 ,actor@127.0.0.1=>   false]",
                          "atype['a'=>13 ,actor@127.0.0.1=>   false]",
                          "btype['a'=>  nat[13] , actor@127.0.0.1=>   abool[false]]",
                          "ctype['a'=>    13 ,  actor@127.0.0.1=>   false]"};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const ptr<Rec> rc1 = Parser::parseObj<Rec>(form);
      TEST_ASSERT_EQUAL(OType::REC, rc1->otype());
      // TEST_ASSERT_EQUAL_INT(13, (rc1->get<Int>(ptr<Str>(new Str("a"))))->value());
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(ptr<Int>(new Int(13)))->otype());
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(ptr<Str>(new Str("no key")))->otype());
      // TEST_ASSERT_FALSE(rc1->get<Bool>(ptr<Uri>(new Uri("actor@127.0.0.1")))->value());
    }

    forms = {"person[[age=>nat[29],name=>'dogturd']]", "person[age=>nat[29],name=>'dogturd']",
             "x@/person[[age=>nat[29],name=>'dogturd']]", "x@/person[age=>nat[29],name=>'dogturd']"};
    for (const string &form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! structure %s", form.c_str());
      const ptr<Rec> rc2 = Parser::parseObj<Rec>(form);
      TEST_ASSERT_EQUAL(OType::REC, rc2->otype());
      TEST_ASSERT_EQUAL_STRING("person", rc2->type()->name().c_str());
      TEST_ASSERT_EQUAL_INT(29, rc2->get<Int>(share<Uri>(Uri("age")))->value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc2->get<Str>(share<Uri>(Uri("name")))->value().c_str());
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(share(Int(13)))->otype()); // TODO
      // TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(share(Str("no key")))->otype());
      TEST_ASSERT_FALSE(rc2->get<Bool>(share(Uri("actor@127.0.0.1")))->value());
    }
    ///////////////////////////////////
    const ptr<Rec> rc2 = Parser::parseObj<Rec>(string("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]"));
    TEST_ASSERT_EQUAL(OType::REC, rc2->otype());
    TEST_ASSERT_EQUAL_INT(13, (rc2->get<Int>(share<Str>(Str("a"))))->value());
    //    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(ptr<Int>(new Int(13)))->otype());
    // TEST_ASSERT_TRUE(rc2->get<Str>(share<Str>(Str("no key")))->isNoObj());
    const ptr<Rec> rc3 = rc2->get<Rec>(share<Uri>(Uri("actor@127.0.0.1")));
    TEST_ASSERT_EQUAL(OType::REC, rc3->otype());
    TEST_ASSERT_EQUAL_INT(1, (rc3->get<Int>(share<Str>(Str("b"))))->value());
    TEST_ASSERT_EQUAL_INT(3, (rc3->get<Int>(share<Str>(Str("c"))))->value());
    TEST_ASSERT_EQUAL_STRING("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]",
                             FOS_DEFAULT_PRINTER::singleton()->strip(rc2->toString().c_str()));
  }

  void test_bcode_parsing() {
    Scheduler<FOS_DEFAULT_ROUTER>::singleton();
    const auto parser = new Parser;
    const ptr<Bytecode> bcode =
        FOS_PRINT_OBJ<Bytecode>(parser->parse("<=(scheduler@kernel?spawn,thread["
                                              "[id    => example,"
                                              " setup => __(0).ref(x).print('setup complete'),"
                                              " loop  => <=(scheduler@kernel?destroy,example)]])"));
    auto process = Processor<Str>(bcode);
    process.forEach([](const ptr<Str> s) { LOG(INFO, "RESULT: %s", s->value().c_str()); });
    delete parser;
    Scheduler<>::singleton()->barrier("wait");
  }

  void test_nested_bytecode_parsing() {
    auto *parser = new Parser();
    const ptr<Bytecode> bcode = FOS_PRINT_OBJ<Bytecode>(parser->parse("plus(mult(plus(3)))"));
    TEST_ASSERT_EQUAL_INT(1, bcode->v_insts()->size());
    //  TEST_ASSERT_EQUAL_INT(1, bcode->v_insts()->at(0)->arg(0)->as<Bytecode>()->v_insts().size());
    //   TEST_ASSERT_EQUAL_INT(
    // 1, bcode->v_insts()->at(0)->arg(0)->as<Bytecode>()->v_insts().at(0)->arg(0)->as<Bytecode>()->v_insts().size());
    delete parser;
  }

  void test_as_parsing() {
    auto *parser = new Parser();
    //  FOS_CHECK_RESULTS<Int>({0}, Fluent(parser->parse("__(0).define(even,mod(2).is(eq(0))")), {}, false);
    //  FOS_CHECK_RESULTS<Int>({Int(32, share<fURI>(fURI("even")))}, Fluent(parser->parse("__(32).as(even)")), {},
    //  false);
    // FOS_CHECK_RESULTS<Uri>({Uri("even")}, Fluent(parser->parse("__(even[32]).as()")), {}, true);
    delete parser;
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
      FOS_RUN_TEST(test_as_parsing); //
  )
}; // namespace fhatos

SETUP_AND_LOOP();


#endif
