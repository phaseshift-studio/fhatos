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

  void test_mono_type_parsing() {
    Parser *parser = new Parser();
    // NOOBJ
    const NoObj *n = parser->parseObj(string("Ø"))->cast<NoObj>();
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->type());
    TEST_ASSERT_EQUAL_STRING("Ø", n->toString().c_str());
    //delete n; can't delete static singleton
    // BOOL
    for (auto pair: List<Pair<string, bool> >({{"true", true}, {"false", false}})) {
      const Bool *b = parser->parseObj(pair.first)->cast<Bool>();
      TEST_ASSERT_EQUAL(OType::BOOL, b->type());
      TEST_ASSERT_EQUAL_INT(pair.second, b->value());
      delete b;
    }
    // INT
    for (auto pair: List<Pair<string, int> >({{"45", 45}, {"0", 0}, {"-12", -12}})) {
      const Int *i = parser->parseObj(pair.first)->cast<Int>();
      TEST_ASSERT_EQUAL(OType::INT, i->type());
      TEST_ASSERT_EQUAL_INT(pair.second, i->value());
      delete i;
    }
    // REAL
    for (auto pair: List<Pair<string, float> >({{"45.54", 45.54}, {"0.0", 0.0}, {"-12.534678", -12.534678}})) {
      const Real *r = parser->parseObj(pair.first)->cast<Real>();
      TEST_ASSERT_EQUAL(OType::REAL, r->type());
      TEST_ASSERT_FLOAT_WITHIN(0.01, pair.second, r->value());
      delete r;
    }
    // STR
    const Str *s = parser->parseObj(string("'fhatty-the-pig'"))->cast<Str>();
    TEST_ASSERT_EQUAL(OType::STR, s->type());
    TEST_ASSERT_EQUAL_STRING("fhatty-the-pig", s->value().c_str());
    delete s;
    // URI
    for (auto pair: List<Pair<string, Uri> >({
             {"fhat://pig", Uri("fhat://pig")},
             {"_2467", Uri("_2467")},
             {".com", Uri(".com")}})) {
      const Uri *u = parser->parseObj(pair.first)->cast<Uri>();
      TEST_ASSERT_EQUAL(OType::URI, u->type());
      TEST_ASSERT_EQUAL_STRING(pair.second.toString().c_str(), u->value().toString().c_str());
      delete u;
    }
    delete parser;
  }

  void test_poly_type_parsing() {
    Parser *parser = new Parser();
    // LST
    // REC
    List<string> forms = {
        "['a'=>13,actor@127.0.0.1=>false]",
        "['a' => 13,actor@127.0.0.1 => false ]",
        "['a'=> 13 , actor@127.0.0.1=>false]",
        "['a' =>    13 , actor@127.0.0.1 =>    false  ]",
        "['a'=>    13 ,actor@127.0.0.1=>   false]"};
    for (const string form: forms) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const Rec *rc1 = parser->parseObj(form)->cast<Rec>();
      TEST_ASSERT_EQUAL(OType::REC, rc1->type());
      TEST_ASSERT_EQUAL_INT(13, (rc1->get<Int>(new Str("a")))->value());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Int(13))->type());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc1->get<Str>(new Str("no key"))->type());
      TEST_ASSERT_FALSE(rc1->get<Bool>(new Uri("actor@127.0.0.1"))->value());
      delete rc1;
    }

    ///////////////////////////////////
    const Rec *rc2 = parser->parseObj(string("['a'=>13,actor@127.0.0.1=>['b'=>1,'c'=>3]]"))->cast<Rec>();
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
    delete parser;
    // INST
    // BYTECODE

  }

  void test_nested_bytecode_parsing() {
    Parser *parser = new Parser();
    const ptr<Bytecode> bcode = FOS_PRINT_OBJ<Bytecode>(parser->parse("__.plus(__.mult(__.plus(3)))"));
    TEST_ASSERT_EQUAL_INT(2, bcode->value()->size());
    TEST_ASSERT_EQUAL_INT(1, bcode->value()->at(1)->
                          arg(0)->as<Bytecode>()->value()->size());
    TEST_ASSERT_EQUAL_INT(1, bcode->value()->at(1)->
                          arg(0)->as<Bytecode>()->value()->at(0)->
                          arg(0)->as<Bytecode>()->value()->size());
    delete parser;
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_no_input_parsing); //
      FOS_RUN_TEST(test_start_inst_parsing); //
      FOS_RUN_TEST(test_basic_parser); //
      FOS_RUN_TEST(test_mono_type_parsing); //
      FOS_RUN_TEST(test_poly_type_parsing); //
      FOS_RUN_TEST(test_nested_bytecode_parsing); //
      )
}; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif
