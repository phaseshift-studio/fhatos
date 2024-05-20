#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#include <test_fhatos.hpp>
//
#include <language/parser.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_basic_parser() {
    Parser parser;
   parser.parse<Int,Int>(new string("plus(1, 2,'hello',  3.246 , true, __.plus(2),4)"));


  }

  FOS_RUN_TESTS(                           //
      FOS_RUN_TEST(test_basic_parser); //
     // FOS_RUN_TEST(test_rec); //
     )
  }; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif