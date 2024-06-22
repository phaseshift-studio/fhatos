#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#include <test_fhatos.hpp>
//
#include <language/obj.hpp>
#include <unity.h>

namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_bool() {
    const BoolP boolA = share(Bool(true, Bool::_t("truth")));
    const BoolP boolB = share(Bool(false, Bool::_t("truth")));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolA, string(boolA->value() ? "true" : "false")).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(*boolA->_type, fURI("/bool/truth"));
    FOS_TEST_OBJ_NOT_EQUAL(boolA, boolB);
    FOS_TEST_OBJ_EQUAL(boolA, boolA);
    FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false, Bool::_t("truth"))), share<Bool>(Bool(*boolA && *boolB)));
    FOS_TEST_OBJ_EQUAL(share(Bool(false)), share<Bool>(Bool(*boolA && *boolB)));
    TEST_ASSERT_TRUE(boolA->value());
    TEST_ASSERT_FALSE(boolA->isBytecode());
    TEST_ASSERT_FALSE(boolA->isNoObj());
    TEST_ASSERT_EQUAL(OType::BOOL, boolA->otype());
    FOS_TEST_OBJ_EQUAL(boolA, boolA->apply(boolB));
    ///
    const BoolP boolBCode = BoolP(new Bool(__().gt(5).bcode, Bool::_t("secret")));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolBCode).c_str());
    FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false)), boolBCode->apply(IntP(new Int(4))));
    FOS_TEST_OBJ_EQUAL(share(Bool(false, Bool::_t("secret"))), boolBCode->apply(IntP(new Int(3 /*,int_t("nat")*/))));
    FOS_TEST_ASSERT_EXCEPTION(share(Bool(__().lt(2).bcode))->apply(share(Bool(true, Bool::_t("truth")))));
  }

  void test_int() {
    const IntP intA = share(Int(1));
    const IntP intB = share<Int>(Int(1));
    const IntP intC = share(Int(1, Int::_t("age")));
    const IntP intD = ptr<Int>(new Int(2, Int::_t("nat")));
    ///
    const IntP int5 = share(Int(5, Int::_t("age")));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(int5, std::to_string(int5->value())).c_str());

    TEST_ASSERT_FALSE(intA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/int", intA->type()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("int", intA->type()->name().c_str());
    TEST_ASSERT_EQUAL(OType::INT, intA->otype());
    TEST_ASSERT_FALSE(intA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*intC->_type, *intA->as("age")->_type);
    FOS_TEST_OBJ_EQUAL(intA, intB);
    FOS_TEST_OBJ_EQUAL(intB, intA);
    FOS_TEST_OBJ_NOT_EQUAL(intB, intB->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intB->as("age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "age"), intB->split(10, "age"));
    FOS_TEST_OBJ_EQUAL(intA, intC->as(""));
    FOS_TEST_OBJ_EQUAL(intA, intA->as(""));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("age"), intC->split(2));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("age"), intB->split(10)->split(2)->as("age"));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(2));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(3)->as("age"));
    /// apply
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intB));
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intA));
    FOS_TEST_OBJ_EQUAL(intA->as("age"), intA->as("age")->apply(intB));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age")->apply(intB));
    /// relations
    FOS_TEST_OBJ_GT(intD, intA->as("nat"));
    FOS_TEST_OBJ_LT(intB->as("nat"), intD);
    ////// BYTECODE
    const IntP intBCode = IntP(new Int(__().plus(Int(0, Int::_t("age"))).bcode, Int::_t("age")));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(intBCode).c_str());
    TEST_ASSERT_TRUE(intBCode->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/int/age", intBCode->type()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("age", intBCode->type()->name().c_str());
    /// apply
    FOS_TEST_OBJ_EQUAL(intC, intBCode->apply(intC));
    FOS_TEST_ASSERT_EXCEPTION(intBCode->apply(share<Int>(Int(2, Int::_t("nat")))))
  }

  void test_str() {
    const StrP strA = share(Str("fhat", Str::_t("first_name")));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(strA).c_str());
    TEST_ASSERT_FALSE(strA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("fhat", strA->value().c_str());
    TEST_ASSERT_EQUAL(OType::STR, strA->otype());
  }

  /* void test_rec() {
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
       TEST_ASSERT_EQUAL_INT32(i, Int(i).v_int());
       TEST_ASSERT_EQUAL_STRING(std::to_string(i).c_str(), Int(i).toString().c_str());
     }
     for (FL_REAL_TYPE r = -100.00f; r < 100.00f; r = r + 0.8024f) {
       TEST_ASSERT_EQUAL_FLOAT(r, Real(r).v_real());
       TEST_ASSERT_FLOAT_WITHIN(0.1f, r, Real(r).v_real());
       TEST_ASSERT_EQUAL_STRING(std::to_string(r).c_str(), Real(r).toString().c_str());
     }
     TEST_ASSERT_EQUAL_STRING("fhatty_fhat_fhat", Str("fhatty_fhat_fhat").toString().c_str());
   }

   void test_rec(){
    Rec rec = Rec{{Str("name"),Str("none")},{Str("title"),Str("dogturd")}};
    ;
    FOS_TEST_MESSAGE("%s",rec.toString().c_str());
    FOS_TEST_MESSAGE("%s",CharSerializer::fromRec(rec).toString().c_str());

   }*/

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_bool); //
      FOS_RUN_TEST(test_int); //
      FOS_RUN_TEST(test_str); //
                              // FOS_RUN_TEST(test_base_obj_strings); //
                              // FOS_RUN_TEST(test_rec); //
                              // FOS_RUN_TEST(test_int); //
  )


}; // namespace fhatos

SETUP_AND_LOOP();


#endif
