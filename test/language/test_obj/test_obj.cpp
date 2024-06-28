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
    const Bool_p boolA = share(Bool(true, "/bool/truth"));
    const Bool_p boolB = share(Bool(false, "truth"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*boolA).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/bool/truth"), *boolA->id());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/bool/truth"), *boolB->id());
    FOS_TEST_OBJ_NOT_EQUAL(boolA, boolB);
    FOS_TEST_OBJ_NOT_EQUAL(boolB, boolA);
    FOS_TEST_OBJ_EQUAL(boolA, boolA);
    FOS_TEST_OBJ_EQUAL(boolB, boolB);
    FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false, "/bool/truth")), share(Bool(*boolA && *boolB)));
    FOS_TEST_OBJ_EQUAL(share(Bool(false)), share(Bool(*boolA && *boolB)));
    TEST_ASSERT_TRUE(boolA->bool_value());
    TEST_ASSERT_FALSE(boolA->isBytecode());
    TEST_ASSERT_FALSE(boolA->isNoObj());
    TEST_ASSERT_EQUAL(OType::BOOL, boolA->o_type());
    FOS_TEST_OBJ_EQUAL(boolA, boolA->apply(boolB));
    ///
    /* const Bool_p boolBCode = share(Bool(__().gt(5).bcode->bcode_value(), "/bcode/secret"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolBCode).c_str());
     FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false)), boolBCode->apply(share(Int(4))));
   //  FOS_TEST_OBJ_EQUAL(share(Bool(false, "/bool/secret")), boolBCode->apply(share(Int(3,int_t("nat")))));
     FOS_TEST_ASSERT_EXCEPTION(__().lt(2).bcode->apply(share(Bool(true, "/bool/truth"))));*/
  }

  void test_int() {
    const Int_p intA = share(Int(1));
    const Int_p intB = share<Int>(Int(1));
    const Int_p intC = share(Int(1, "/int/age"));
    const Int_p intD = ptr<Int>(new Int(2, "nat"));
    ///
    const Int_p int5 = share(Int(5, "age"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*int5).c_str());

    TEST_ASSERT_FALSE(intA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/int/", intA->id()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("", intA->id()->lastSegment().c_str());
    TEST_ASSERT_EQUAL_STRING("int", intA->id()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL(OType::INT, intA->o_type());
    TEST_ASSERT_FALSE(intA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*intC->id(), *intA->as(share(fURI("age")))->id());
    FOS_TEST_OBJ_EQUAL(intA, intB);
    FOS_TEST_OBJ_EQUAL(intB, intA);
    FOS_TEST_OBJ_NOT_EQUAL(intB, intB->as("/int/age"));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intB->as("/int/age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/int/age"), intB->split(10, "age"));
    FOS_TEST_OBJ_EQUAL(intA, intC->as("/int/"));
    FOS_TEST_OBJ_EQUAL(intA, intA->as("/int/"));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("age"), intC->split(2));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("/int/age"), intB->split(10)->split(2)->as("age"));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(2));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(3)->as("/int/age"));
    /// apply
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intB));
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intA));
    FOS_TEST_OBJ_EQUAL(intA->as("age"), intA->as("/int/age")->apply(intB));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age")->apply(intB));
    /// relations
    FOS_TEST_OBJ_GT(intD, intA->as("nat"));
    FOS_TEST_OBJ_LT(intB->as("/int/nat"), intD);
    ////// BYTECODE
    /* const Int_p intBCode = share(Int(__().plus(Int(0, "/int/age")).bcode->bcode_value(), "/bcode/age"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(intBCode).c_str());
     // TEST_ASSERT_TRUE(intBCode->isBytecode());
     TEST_ASSERT_EQUAL_STRING("/bcode/age", intBCode->id()->toString().c_str());
     TEST_ASSERT_EQUAL_STRING("age", intBCode->id()->lastSegment().c_str());
     /// apply
     FOS_TEST_OBJ_EQUAL(intC, intBCode->apply(intC));
     FOS_TEST_ASSERT_EXCEPTION(intBCode->apply(share<Int>(Int(2, "/nat"))))*/
  }

  void test_str() {
    const Str strA = Str("fhat", "first_name");
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(strA).c_str());
    TEST_ASSERT_FALSE(strA.isBytecode());
    TEST_ASSERT_EQUAL_STRING("fhat", strA.str_value().c_str());
    TEST_ASSERT_EQUAL(OType::STR, strA.o_type());
  }

  void test_rec() {
    const Rec recA = Rec({{"a", 1}, {"b", 2}});
    const Rec recB = Rec({{"a", 1}, {"b", 2}});
    const Rec recC = Rec({{Str("a", "letter"), 1}, {Str("b", "letter"), 2}}, "mail");
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(recC).c_str());
    TEST_ASSERT_TRUE(recA == recB);
    TEST_ASSERT_FALSE(recA != recB);
    TEST_ASSERT_FALSE(recA == recC);
    TEST_ASSERT_TRUE(recA != recC);
    ////
    TEST_ASSERT_EQUAL_INT(1, recA.rec_get("a")->int_value());
    TEST_ASSERT_EQUAL_INT(2, recA.rec_get("b")->int_value());
    TEST_ASSERT_TRUE(recA.rec_get("c")->isNoObj());
    recA.rec_set("b", 12);
    TEST_ASSERT_EQUAL_INT(12, recA.rec_get("b")->int_value());
    recA.rec_set("b", Real(202.5f, "cost"));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 202.5f, recA.rec_get("b")->real_value());
    recA.rec_delete("b");
    TEST_ASSERT_TRUE(recA.rec_get("b")->isNoObj());
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(recA).c_str());
  }

  /*void test_base_obj_strings() {
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

  void test_inst_bcode() {
    Fluent f = __(*BCode::to_bcode({Insts::plus(share(Int(1)))})).plus(2).mult(3);
    f.forEach<Obj>([](const Obj_p &x) { LOG(INFO, "%s\n", x->toString().c_str()); });
  }

  FOS_RUN_TESTS( //
      Obj::Types<>::addToCache(share(fURI("/bool/truth")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/int/age")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/int/nat")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/str/first_name")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/str/letter")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/rec/mail")), Insts::NO_OP_BCODE());
      Obj::Types<>::addToCache(share(fURI("/real/cost")), Insts::NO_OP_BCODE());

      FOS_RUN_TEST(test_bool); //
      FOS_RUN_TEST(test_int); //
      FOS_RUN_TEST(test_str); //
      FOS_RUN_TEST(test_rec); //
      // FOS_RUN_TEST(test_inst_bcode); //
  )


}; // namespace fhatos

SETUP_AND_LOOP();


#endif
