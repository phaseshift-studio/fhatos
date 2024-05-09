#ifndef fhatos_kernel__test_fluent_hpp
#define fhatos_kernel__test_fluent_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/language/fluent.hpp>
#include <kernel/language/obj.hpp>
#include <unity.h>

namespace fhatos::kernel {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void test_fluent() {
  const Fluent<Int, Int> f =
      Fluent<Int, Int>(Int(10)).plus(Int(20)).plus(Int(5));
  Serial.println("HERE");
  FOS_TEST_MESSAGE("%s\n", f.toString().c_str());
}

FOS_RUN_TESTS(                 //
    FOS_RUN_TEST(test_fluent); //

);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif