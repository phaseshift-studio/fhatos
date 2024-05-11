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

 Fluent<Int, Int> __( Int start) { return Fluent<Int, Int>(start); }

void test_fluent() {
  FOS_TEST_MESSAGE("%s\n", __<Int>(10).plus(Int(20)).plus(Int(5)).toString().c_str());
  FOS_TEST_MESSAGE("%s\n", __<Int>(10).plus(__<Int>().plus(6).plus(12).plus(__<Int>().plus(13).plus(6))).toString().c_str());
  FOS_TEST_MESSAGE("%s\n", Bytecode<Int,Int>(Int(10)).toString().c_str());
}

FOS_RUN_TESTS(                 //
    FOS_RUN_TEST(test_fluent); //

);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif