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

const Fluent<Int, Int> __(const Int start) { return Fluent<Int, Int>(start); }

void test_fluent() {
  FOS_TEST_MESSAGE("%s\n",
                   __(10).plus(20).plus(5).toString().c_str());
}

FOS_RUN_TESTS(                 //
    FOS_RUN_TEST(test_fluent); //

);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif