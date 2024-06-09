#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#include <test_fhatos.hpp>
//
#include <language/parser.hpp>
#include <language/processor.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_basic_parser() {
    try {
      Parser parser;
      parser.parseToFluent("__(actor@127.9.9.1).<=(__.plus(temp),'hello').plus(dmc).=>(__,__)")->forEach<Uri>(
          [](const Uri *uri) {
            LOG(INFO, "==>%s\n", uri->value().toString().c_str());
          });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int>(parser.parse("__(15).plus(__)")).forEach([](const Int *i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int> *p = new Processor<Int>(parser.parse("__(15).plus(1).plus(__.plus(5))"));
      p->forEach([](const Int *i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
    } catch (fError error) {
      LOG_EXCEPTION(error);
    }
  }

  void test_no_input_parsing() {
    Parser *parser = new Parser();
    ptr<Bytecode> bcode = parser->parse("");
    TEST_ASSERT_EQUAL_INT(0, bcode->value()->size());
    delete parser;
  }

  void test_start_inst_parsing() {
    Parser *parser = new Parser();
    FOS_CHECK_ARGS<Int>({new Int(15)}, parser->parse("__(15)")->startInst());
    FOS_CHECK_ARGS<Int>({new Int(15), new Int(-10), new Int(0)}, parser->parse("__(15,-10,0)")->startInst());
    FOS_CHECK_ARGS<Obj>({new Str("fhat"), new Str("os"), new Int(69)},
                        parser->parse("__('fhat','os',69)")->startInst());
    FOS_CHECK_ARGS<Obj>({new Rec({{new Str("a"), new Int(2)}})},
                        parser->parse("__(['a'=>2])")->startInst());
    FOS_CHECK_ARGS<Obj>({new Rec(
                        {{new Str("a"), new Int(2)},
                         {new Str("b"), new Int(3)}})},
                        parser->parse("__(['a'=>2,'b'=>3])")->startInst());
    FOS_CHECK_ARGS<Obj>({new Rec({
                            {new Str("a"), new Int(2)},
                            {new Str("b"), new Rec(
                                 {{new Int(3), new Uri("http://fhatos.org")}})}})},
                        parser->parse("__(['a'=>2,'b'=>[3=>http://fhatos.org]])")->startInst());
    delete parser;
  }

  void test_noobj_parsing() {
    const NoObj *n = Parser::parseObj(string("Ø"))->cast<NoObj>();
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->type());
    TEST_ASSERT_EQUAL_STRING("Ø", n->toString().c_str());
    //delete n; can't delete static singleton
  }

  void test_bool_parsing() {
    for (auto pair: List<Pair<string, bool> >({{"true", true}, {"false", false}})) {
      const Bool *b = Parser::parseObj(pair.first)->cast<Bool>();
      TEST_ASSERT_EQUAL(OType::BOOL, b->type());
      TEST_ASSERT_EQUAL_INT(pair.second, b->value());
      delete b;
    }
  }

  void test_int_parsing() {
    for (auto pair: List<Pair<string, int> >({{"45", 45}, {"0", 0}, {"-12", -12}})) {
      const Int *i = Parser::parseObj(pair.first)->cast<Int>();
      TEST_ASSERT_EQUAL(OType::INT, i->type());
      TEST_ASSERT_EQUAL_INT(pair.second, i->value());
      delete i;
    }
    // TYPED INT
    for (auto pair: List<Pair<string, Int> >({{"nat[100]", Int(100, share(fURI("nat")))},
                                              {"zero[0]", Int(0, share(fURI("zero")))},
                                              {"z[-100]", Int(-100, share(fURI("z")))}})) {
      const Int *i = Parser::parseObj(pair.first)->cast<Int>();
      TEST_ASSERT_EQUAL(OType::INT, i->type());
      TEST_ASSERT_EQUAL_STRING(pair.second.utype()->toString().c_str(), i->utype()->toString().c_str());
      TEST_ASSERT_EQUAL_INT(pair.second.value(), i->value());
      delete i;
    }
  }

  void test_real_parsing() {
    // REAL
    for (auto pair: List<Pair<string, float> >({{"45.54", 45.54}, {"0.0", 0.0}, {"-12.534678", -12.534678}})) {
      const Real *r = Parser::parseObj(pair.first)->cast<Real>();
      TEST_ASSERT_EQUAL(OType::REAL, r->type());
      TEST_ASSERT_FLOAT_WITHIN(0.01, pair.second, r->value());
      delete r;
    }
  }

  void test_uri_parsing() {
    for (auto pair: List<Pair<string, Uri> >({
             {"fhat://pig", Uri("fhat://pig")},
             {"_2467", Uri("_2467")},
             {".com", Uri(".com")}})) {
      const Uri *u = Parser::parseObj(pair.first)->cast<Uri>();
      TEST_ASSERT_EQUAL(OType::URI, u->type());
      TEST_ASSERT_EQUAL_STRING(pair.second.toString().c_str(), u->value().toString().c_str());
      delete u;
    }
  }

  void test_str_parsing() {
    const Str *s = Parser::parseObj(string("'fhatty-the-pig'"))->cast<Str>();
    TEST_ASSERT_EQUAL(OType::STR, s->type());
    TEST_ASSERT_EQUAL_STRING("fhatty-the-pig", s->value().c_str());
    delete s;
  }

  void test_rec_parsing() {
    // REC
    List<string> forms = {
        "['a'=>13,actor@127.0.0.1=>false]",
        "['a' => 13,actor@127.0.0.1 => false ]",
        "['a'=> 13 , actor@127.0.0.1=>false]",
        "['a' =>    13 , actor@127.0.0.1 =>    false  ]",
        "['a'=>    13 ,actor@127.0.0.1=>   false]"};
    for (const string form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const Rec *rc1 = Parser::parseObj(form)->cast<Rec>();
      TEST_ASSERT_EQUAL(OType::REC, rc1->type());
      TEST_ASSERT_EQUAL_INT(13, (rc1->get<Int>(new Str("a")))->value());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Int(13))->type());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Str("no key"))->type());
      TEST_ASSERT_FALSE(rc1->get<Bool>(new Uri("actor@127.0.0.1"))->value());
      delete rc1;
    }

    forms = {
        "person[age=>nat[29],name=>'dogturd']"};
    for (const string form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const Rec *rc1 = Parser::parseObj(form)->cast<Rec>();
      TEST_ASSERT_EQUAL(OType::REC, rc1->type());
      TEST_ASSERT_EQUAL_STRING("person", rc1->utype()->toString().c_str());
      TEST_ASSERT_EQUAL_INT(29, (rc1->get<Int>(new Uri("age")))->value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc1->get<Str>(new Uri("name"))->value().c_str());
      //TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Int(13))->type());
      //TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Str("no key"))->type());
      //TEST_ASSERT_FALSE(rc1->get<Bool>(new Uri("actor@127.0.0.1"))->value());
      delete rc1;
    }

    ///////////////////////////////////
    const Rec *rc2 = Parser::parseObj(string("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]"))->cast<Rec>();
    TEST_ASSERT_EQUAL(OType::REC, rc2->type());
    TEST_ASSERT_EQUAL_INT(13, (rc2->get<Int>(new Str("a")))->value());
    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(new Int(13))->type());
    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(new Str("no key"))->type());
    const Rec *rc3 = rc2->get<Rec>(new Uri("actor@127.0.0.1"));
    TEST_ASSERT_EQUAL(OType::REC, rc3->type());
    TEST_ASSERT_EQUAL_INT(1, (rc3->get<Int>(new Str("b")))->value());
    TEST_ASSERT_EQUAL_INT(3, (rc3->get<Int>(new Str("c")))->value());
    // TODO: strip ansi TEST_ASSERT_EQUAL_STRING("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]",rc2->toString().c_str());
    delete rc2;
    delete rc3;
  }

  void test_poly_type_parsing() {

    // LST

    // INST
    // BYTECODE

  }

  void test_nested_bytecode_parsing() {
    Parser *parser = new Parser();
    const ptr<Bytecode> bcode = FOS_PRINT_OBJ<Bytecode>(parser->parse("plus(mult(plus(3)))"));
    TEST_ASSERT_EQUAL_INT(1, bcode->value()->size());
    TEST_ASSERT_EQUAL_INT(1, bcode->value()->at(0)->
                          arg(0)->as<Bytecode>()->value()->size());
    TEST_ASSERT_EQUAL_INT(1, bcode->value()->at(0)->
                          arg(0)->as<Bytecode>()->value()->at(0)->
                          arg(0)->as<Bytecode>()->value()->size());
    delete parser;
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_no_input_parsing); //
      FOS_RUN_TEST(test_start_inst_parsing); //
      FOS_RUN_TEST(test_basic_parser); //
      FOS_RUN_TEST(test_noobj_parsing); //
      FOS_RUN_TEST(test_bool_parsing); //
      FOS_RUN_TEST(test_int_parsing); //
      FOS_RUN_TEST(test_real_parsing); //
      FOS_RUN_TEST(test_uri_parsing); //
      FOS_RUN_TEST(test_str_parsing); //
      FOS_RUN_TEST(test_rec_parsing); //
      FOS_RUN_TEST(test_nested_bytecode_parsing); //
      )
}; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif
