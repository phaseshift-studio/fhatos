#ifndef fhatos_kernel__test_obj_hpp
#define fhatos_kernel__test_obj_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/language/obj.hpp>
#include <unity.h>

namespace fhatos::kernel {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void test_base_obj_strings() {
  TEST_ASSERT_EQUAL_STRING("true", Bool(true).toString().c_str());
  TEST_ASSERT_EQUAL_STRING("false", Bool(false).toString().c_str());
  for (FL_INT_TYPE i = -100; i < 100; i++) {
    TEST_ASSERT_EQUAL_INT32(i, Int(i).get());
    TEST_ASSERT_EQUAL_STRING((String() + i).c_str(), Int(i).toString().c_str());
  }
  for (FL_REAL_TYPE r = -100.00f; r < 100.00f; r = r + 0.8024f) {
    TEST_ASSERT_EQUAL_FLOAT(r, Real(r).get());
    // TEST_ASSERT_EQUAL_DOUBLE(r,Real(r).get());
    TEST_ASSERT_EQUAL_STRING(String(r).c_str(), Real(r).toString().c_str());
    TEST_ASSERT_EQUAL((String(r)).toFloat(), Real(r).toString().toFloat());
  }
  TEST_ASSERT_EQUAL_STRING("fhatty_fhat_fhat",
                           Str("fhatty_fhat_fhat").toString().c_str());
}

FOS_RUN_TESTS(                           //
    FOS_RUN_TEST(test_base_obj_strings); //

)

}; // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif