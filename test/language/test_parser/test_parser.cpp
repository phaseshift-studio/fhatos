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
      parser.parseToFluent<Uri, Uri>("__(actor@127.9.9.1).<=(__.plus(temp),'hello').plus(dmc).=>(__,__)")->forEach(
          [](const Uri *uri) {
            LOG(INFO, "==>%s\n", uri->value().toString().c_str());
          });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int, Int>(parser.parse<Int, Int>("__(15).plus(__)")).forEach([](const Int *i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
      FOS_TEST_MESSAGE("=========================\n");
      Processor<Int, Int> *p = new Processor<Int, Int>(parser.parse<Int, Int>("__(15).plus(1).plus(__.plus(5))"));
      p->forEach([](const Int *i) {
        LOG(INFO, "==>%s\n", i->toString().c_str());
      });
    } catch (fError error) {
      LOG_EXCEPTION(error);
    }
  }

  void test_mono_type_parsing() {
    Parser *parser = new Parser();
    // BOOL
    const Bool *b = parser->parseObj<Bool>(string("true"))->cast<Bool>();
    TEST_ASSERT_EQUAL(BOOL, b->type());
    TEST_ASSERT_TRUE(b->value());
    delete b;
    // INT
    const Int *i = parser->parseObj<Int>(string("24"))->cast<Int>();
    TEST_ASSERT_EQUAL(INT, i->type());
    TEST_ASSERT_EQUAL_INT(24, i->value());
    delete i;
    // REAL
    const Real *r = parser->parseObj<Real>(string("45.54"))->cast<Real>();
    TEST_ASSERT_EQUAL(REAL, r->type());
    TEST_ASSERT_FLOAT_WITHIN(0.1, 45.54, r->value());
    delete r;
    // STR
    // URI
    const Uri *u = parser->parseObj<Uri>(string("fhat@127.0.0.1/os"))->cast<Uri>();
    TEST_ASSERT_EQUAL(URI, u->type());
    TEST_ASSERT_EQUAL_STRING("fhat@127.0.0.1/os", u->value().toString().c_str());
    delete u;
    delete parser;
  }

  void test_poly_type_parsing() {
    Parser *parser = new Parser();
    // LST
    // REC
    const Rec *rc = parser->parseObj<Rec>(string("['a',13,actor@127.0.0.1,false]"))->cast<Rec>();
    TEST_ASSERT_EQUAL(REC, rc->type());
    TEST_ASSERT_EQUAL_INT(13, (rc->get<Int>(new Str("a")))->value());
    TEST_ASSERT_TRUE(rc->get<Str>(new Int(13)) == nullptr);
    TEST_ASSERT_TRUE(rc->get<Str>(new Str("no key")) == nullptr);
    TEST_ASSERT_FALSE(rc->get<Bool>(new Uri("actor@127.0.0.1"))->value());
    delete rc;
    delete parser;
    // INST
    // BYTECODE

  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_basic_parser); //
      FOS_RUN_TEST(test_mono_type_parsing); //
      FOS_RUN_TEST(test_poly_type_parsing); //
      // FOS_RUN_TEST(test_rec); //
      )
}; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif
