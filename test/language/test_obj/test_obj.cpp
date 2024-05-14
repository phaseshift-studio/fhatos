#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#include <test_fhatos.hpp>
//
#include <language/obj.hpp>
#include <language/binary_obj.hpp>

namespace fhatos {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void test_base_obj_strings() {
  TEST_ASSERT_EQUAL_STRING("true", Bool(true).toString().c_str());
  TEST_ASSERT_EQUAL_STRING("false", Bool(false).toString().c_str());
  for (FL_INT_TYPE i = -100; i < 100; i++) {
    TEST_ASSERT_EQUAL_INT32(i, Int(i).value());
    TEST_ASSERT_EQUAL_STRING(std::to_string(i).c_str(), Int(i).toString().c_str());
  }
  for (FL_REAL_TYPE r = -100.00f; r < 100.00f; r = r + 0.8024f) {
    TEST_ASSERT_EQUAL_FLOAT(r, Real(r).value());
     TEST_ASSERT_FLOAT_WITHIN(0.1f,r,Real(r).value());
    TEST_ASSERT_EQUAL_STRING(std::to_string(r).c_str(), Real(r).toString().c_str());
  }
  TEST_ASSERT_EQUAL_STRING("fhatty_fhat_fhat",
                           Str("fhatty_fhat_fhat").toString().c_str());
}

/*void test_rec(){
 Rec rec = Rec{{Str("name"),Str("none")},{Str("title"),Str("dogturd")}};
 ;
 FOS_TEST_MESSAGE("%s",rec.toString().c_str());
 FOS_TEST_MESSAGE("%s",CharSerializer::fromRec(rec).toString().c_str());

}*/

FOS_RUN_TESTS(                           //
    FOS_RUN_TEST(test_base_obj_strings); //
   // FOS_RUN_TEST(test_rec); //
   )



}; // namespace fhatos::kernel

SETUP_AND_LOOP();


#endif