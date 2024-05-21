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
    Parser parser;
   LOG(INFO,"RESULT: %s\n\n",parser.parse<Int,Int>(new string("__(12,15,6).plus(10)"))->toString().c_str());

   Processor<Int,Int>* p = new Processor<Int,Int>(parser.parse<Int,Int>(new string("__(15).plus(1).plus(__.plus(5))")));
   p->forEach([](const Int* i) {
     LOG(INFO,"==>%s\n",i->toString().c_str());
   });



  }

  FOS_RUN_TESTS(                           //
      FOS_RUN_TEST(test_basic_parser); //
     // FOS_RUN_TEST(test_rec); //
     )
  }; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif