#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#include <test_fhatos.hpp>
//
#include <language/binary_obj.hpp>
#include <language/obj.hpp>
#include <unity.h>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_int() {
    const Int intA = Int(1);
    const Int intB = Int(1);
    const Int intC = Int(1, share<fURI>(fURI("age")));
    FOS_TEST_OBJ_EQUAL(&intA, &intB);
    FOS_TEST_OBJ_EQUAL(&intB, &intA);
    FOS_TEST_OBJ_NOT_EQUAL(&intB, intB.cast("age"));
    FOS_TEST_OBJ_EQUAL(&intC, intC.cast("age"));
    FOS_TEST_OBJ_EQUAL(&intC, intA.cast("age"));
    FOS_TEST_OBJ_EQUAL(&intA, intC.cast());
    FOS_TEST_OBJ_EQUAL(&intA, intA.cast());
    FOS_TEST_OBJ_EQUAL(intA.split(2)->cast("age"), intC.split(2));
    FOS_TEST_OBJ_EQUAL(intA.split(2)->cast("age"), intB.split(10)->split(2)->cast("age"));
    FOS_TEST_OBJ_NOT_EQUAL(intA.split(2)->cast("age"), intB.split(2));
    FOS_TEST_OBJ_NOT_EQUAL(intA.split(2)->cast("age"), intB.split(3)->cast("age"));
  }

  void test_rec() {
    const Rec recA = Rec({{new Str("a"), new Int(1)}, {new Str("b"), new Int(2)}});
    const Rec recB = Rec({{new Str("a"), new Int(1)}, {new Str("b"), new Int(2)}});
    const Rec recC = Rec({{new Str("a", share<fURI>(fURI("letter"))), new Int(1)},
                          {new Str("b", share<fURI>(fURI("letter"))), new Int(2)}});
    TEST_ASSERT_TRUE(recA == recB);
    TEST_ASSERT_FALSE(recA == recC);
  }

  void test_base_obj_strings() {
    TEST_ASSERT_EQUAL_STRING("true", Bool(true).toString().c_str());
    TEST_ASSERT_EQUAL_STRING("false", Bool(false).toString().c_str());
    for (FL_INT_TYPE i = -100; i < 100; i++) {
      TEST_ASSERT_EQUAL_INT32(i, Int(i).value());
      TEST_ASSERT_EQUAL_STRING(std::to_string(i).c_str(), Int(i).toString().c_str());
    }
    for (FL_REAL_TYPE r = -100.00f; r < 100.00f; r = r + 0.8024f) {
      TEST_ASSERT_EQUAL_FLOAT(r, Real(r).value());
      TEST_ASSERT_FLOAT_WITHIN(0.1f, r, Real(r).value());
      TEST_ASSERT_EQUAL_STRING(std::to_string(r).c_str(), Real(r).toString().c_str());
    }
    TEST_ASSERT_EQUAL_STRING("fhatty_fhat_fhat", Str("fhatty_fhat_fhat").toString().c_str());
  }

  /*\void test_rec(){
   Rec rec = Rec{{Str("name"),Str("none")},{Str("title"),Str("dogturd")}};
   ;
   FOS_TEST_MESSAGE("%s",rec.toString().c_str());
   FOS_TEST_MESSAGE("%s",CharSerializer::fromRec(rec).toString().c_str());

  }*/

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_base_obj_strings); //
      FOS_RUN_TEST(test_rec); //
      FOS_RUN_TEST(test_int); //
  )


}; // namespace fhatos

SETUP_AND_LOOP();


#endif
