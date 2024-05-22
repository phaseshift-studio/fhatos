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
   parser.parseToFluent<Uri,Uri>("__(actor@127.9.9.1).<=('hello').plus(dmc).=>(abc@123.4.5.6,__.plus('hey'))")->forEach([](const Uri* uri) {
     LOG(INFO,"==>%s\n",uri->value().toString().c_str());
   });

   Processor<Int,Int>* p = new Processor<Int,Int>(parser.parse<Int,Int>("__(15).plus(1).plus(__.plus(5))"));
   p->forEach([](const Int* i) {
     LOG(INFO,"==>%s\n",i->toString().c_str());
   });
} catch(fError error) {
  LOG_EXCEPTION(error);
}
  }

  FOS_RUN_TESTS(                           //
      FOS_RUN_TEST(test_basic_parser); //
     // FOS_RUN_TEST(test_rec); //
     )
  }; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif