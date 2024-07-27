#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>
namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_bool() {
    Types::singleton()->saveType(id_p("/bool/truth"), Obj::to_bcode({})); //
    const Bool_p boolA = share(Bool(true, "/bool/truth"));
    const Bool_p boolB = share(Bool(false, "truth"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*boolA).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/bool/truth"), *boolA->id());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/bool/truth"), *boolB->id());
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
    TEST_ASSERT_TRUE(boolA->match(boolA));
    TEST_ASSERT_TRUE(Obj::to_bool(true)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({}))})));
    TEST_ASSERT_FALSE(Obj::to_bool(false)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({}))})));
    /* const Bool_p boolBCode = share(Bool(__().gt(5).bcode->bcode_value(), "/bcode/secret"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolBCode).c_str());
     FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false)), boolBCode->apply(share(Int(4))));
   //  FOS_TEST_OBJ_EQUAL(share(Bool(false, "/bool/secret")), boolBCode->apply(share(Int(3,int_t("nat")))));
     FOS_TEST_ASSERT_EXCEPTION(__().lt(2).bcode->apply(share(Bool(true, "/bool/truth"))));*/
  }

  void test_int() {
    Types::singleton()->saveType(id_p("/int/age"), Obj::to_bcode({})); //
    Types::singleton()->saveType(id_p("/int/nat"), Obj::to_bcode({})); //
    const Int_p intA = share(Int(1));
    const Int_p intB = share<Int>(Int(1));
    const Int_p intC = share(Int(1, "/int/age"));
    const Int_p intD = ptr<Int>(new Int(2, "nat"));
    ///
    const Int_p int5 = share(Int(5, "age"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*int5).c_str());

    TEST_ASSERT_FALSE(intA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/int/", intA->id()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("", intA->id()->name());
    TEST_ASSERT_EQUAL_STRING("int", intA->id()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL(OType::INT, intA->o_type());
    TEST_ASSERT_FALSE(intA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*intC->id(), *intA->as(id_p("age"))->id());
    FOS_TEST_OBJ_EQUAL(intA, intB);
    FOS_TEST_OBJ_EQUAL(intB, intA);
    FOS_TEST_OBJ_NOT_EQUAL(intB, intB->as("/int/age"));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intB->as("/int/age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/int/age"), intB->split(10, id_p("/int/age")));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/int/age"), intB->split(10, "age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/int/age"), intB->split(10, "/int/age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "age"), intB->split(10, "age"));
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
    TEST_ASSERT_GREATER_THAN_INT(intD->int_value(), intA->as("nat")->int_value());
    TEST_ASSERT_LESS_THAN_INT(intB->as("/int/nat")->int_value(), intD->int_value());
    /// match
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_int(22)));
    TEST_ASSERT_TRUE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("nat")));
    TEST_ASSERT_FALSE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("age")));
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(0))}))})));
    TEST_ASSERT_FALSE(Obj::to_int(22)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(23))}))})));
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

  void test_real() {
    Types::singleton()->saveType(id_p("/real/money"), Obj::to_bcode({})); //
    Types::singleton()->saveType(id_p("/real/weight"), Obj::to_bcode({})); //
    const Real_p realA = share(Real(1.0f));
    const Real_p realB = share<Real>(Real(1.0f));
    const Real_p realC = share(Real(1.0f, "/real/money"));
    const Real_p realD = ptr<Real>(new Real(2.0f, "weight"));
    ///
    const Real_p real5 = share(Real(5.0f, "money"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*real5).c_str());

    TEST_ASSERT_FALSE(realA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/real/", realA->id()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("", realA->id()->name());
    TEST_ASSERT_EQUAL_STRING("real", realA->id()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL(OType::REAL, realA->o_type());
    TEST_ASSERT_FALSE(realA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*realC->id(), *realA->as(id_p("money"))->id());
    FOS_TEST_OBJ_EQUAL(realA, realB);
    FOS_TEST_OBJ_EQUAL(realB, realA);
    FOS_TEST_OBJ_NOT_EQUAL(realB, realB->as("/real/money"));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money"));
    FOS_TEST_OBJ_EQUAL(realC, realB->as("/real/money"));
    FOS_TEST_OBJ_EQUAL(realA->split(10.0f, "/real/money"), realB->split(10.0f, "money"));
    FOS_TEST_OBJ_EQUAL(realA, realC->as("/real/"));
    FOS_TEST_OBJ_EQUAL(realA, realA->as("/real/"));
    FOS_TEST_OBJ_EQUAL(realA->split(2.0f)->as("money"), realC->split(2.0f));
    FOS_TEST_OBJ_EQUAL(realA->split(2.0f)->as("/real/money"), realB->split(10.0f)->split(2.0f)->as("money"));
    FOS_TEST_OBJ_NOT_EQUAL(realA->split(2.0f)->as("money"), realB->split(2.0f));
    FOS_TEST_OBJ_NOT_EQUAL(realA->split(2.0f)->as("money"), realB->split(3.0f)->as("/real/money"));
    /// apply
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realB));
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realA));
    FOS_TEST_OBJ_EQUAL(realA->as("money"), realA->as("/real/money")->apply(realB));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money")->apply(realB));
    /// relations
    TEST_ASSERT_GREATER_THAN_FLOAT(realD->real_value(), realA->as("weight")->real_value());
    TEST_ASSERT_LESS_THAN_FLOAT(realB->as("/real/weight")->real_value(), realD->real_value());
    /// match
    TEST_ASSERT_TRUE(Obj::to_real(22.1f)->match(Obj::to_real(22.1f)));
    TEST_ASSERT_TRUE(Obj::to_real(22.1f)->as("weight")->match(Obj::to_real(22.1f)->as("weight")));
    TEST_ASSERT_FALSE(Obj::to_real(22.1f)->as("money")->match(Obj::to_real(22.1f)->as("weight")));
    TEST_ASSERT_TRUE(
        Obj::to_real(22.1f)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_real(0.1f))}))})));
    TEST_ASSERT_FALSE(
        Obj::to_real(22.1f)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_real(23.1f))}))})));
  }

  void test_str() {
    Types::singleton()->saveType(id_p("/str/first_name"), Obj::to_bcode({})); //
    Types::singleton()->saveType(id_p("/str/letter"), Obj::to_bcode({})); //
    const Str strA = *Obj::to_str("fhat", id_p("/str/first_name"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(strA).c_str());
    TEST_ASSERT_FALSE(strA.isBytecode());
    TEST_ASSERT_EQUAL_STRING("fhat", strA.str_value().c_str());
    TEST_ASSERT_EQUAL(OType::STR, strA.o_type());
  }

  void test_lst() {
    Types::singleton()->saveType(id_p("/lst/ones"), Obj::to_lst({1, 1, 1}));
    const Lst lstA = *Obj::to_lst({1, 2, 3, 4});
    const Lst lstB = *Obj::to_lst({1, 2, 3, 4});
    const Lst lstC = *Obj::to_lst({2, 3, 4});
    FOS_TEST_OBJ_EQUAL(&lstA, &lstB);
    FOS_TEST_OBJ_EQUAL(&lstB, &lstA);
    FOS_TEST_OBJ_NOT_EQUAL(&lstA, &lstC);
    FOS_TEST_OBJ_NOT_EQUAL(&lstB, &lstC);
    TEST_ASSERT_EQUAL_INT(4, lstA.lst_value()->size());
    TEST_ASSERT_EQUAL_INT(4, lstB.lst_value()->size());
    TEST_ASSERT_EQUAL_INT(3, lstC.lst_value()->size());
    lstC.lst_set(o_p(0), o_p(1));
    FOS_TEST_OBJ_EQUAL(&lstC, &lstA);
    lstC.lst_set(o_p(4), o_p(5));
    FOS_TEST_OBJ_NOT_EQUAL(&lstC, &lstA);
    for (int i = 0; i < 4; i++) {
      FOS_TEST_OBJ_EQUAL(o_p(i + 1), lstA.lst_get(o_p(i)));
      FOS_TEST_OBJ_EQUAL(o_p(i + 1), lstB.lst_get(o_p(i)));
      FOS_TEST_OBJ_EQUAL(o_p(i + 1), lstC.lst_get(o_p(i)));
    }
    const Lst_p lstD = Obj::to_lst({1, 1, 1});
    TEST_ASSERT_EQUAL_STRING("/lst/ones", lstD->as("/lst/ones")->id()->toString().c_str());
    try {
      Obj_p x = lstA.as("/lst/ones");
      TEST_FAIL_MESSAGE("Should throw exception");
    } catch (const fError &) {
      TEST_ASSERT_TRUE(true);
    }
  }

  void test_rec() {
    Types::singleton()->saveType(id_p("/rec/mail"), Obj::to_bcode({})); //
    Types::singleton()->saveType(id_p("/real/cost"), Obj::to_bcode({})); //
    const Rec recA = *Obj::to_rec({{"a", 1}, {"b", 2}});
    const Rec recB = *Obj::to_rec({{"a", 1}, {"b", 2}});
    const Rec recC =
        Obj(share(Obj::RecMap<>({make_pair<const Obj_p, Obj_p>(Obj::to_str("a", id_p("/str/letter")), share(Int(1))),
                                 make_pair<const Obj_p, Obj_p>(Obj::to_str("b", id_p("/str/letter")), share(Int(2)))})),
            id_p("/rec/mail"));
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
    // match
    TEST_ASSERT_TRUE(Obj::to_rec({{"a", 1}, {"b", 2}})->match(Obj::to_rec({{"a", 1}, {"b", 2}})));
    TEST_ASSERT_FALSE(Obj::to_rec({{"a", 1}, {"b", 2}})->match(Obj::to_rec({{"a", 1}, {"b", 2}, {"c", 3}})));
    TEST_ASSERT_TRUE(
        Obj::to_rec({{"a", 1}, {"b", 2}})
            ->match(Obj::to_rec(
                {{"a", *Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(0))}))})}, {"b", 2}})));
    TEST_ASSERT_FALSE(
        Obj::to_rec({{"a", 1}, {"b", 2}})
            ->match(Obj::to_rec(
                {{"a", *Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(3))}))})}, {"b", 2}})));
    TEST_ASSERT_TRUE(
        Obj::to_rec({{"a", 1}, {"b", 2}})
            ->match(Obj::to_rec({{"a", *Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(-10))}))})},
                                 {*Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::eq(Obj::to_str("b"))}))}), 2}})));
    TEST_ASSERT_FALSE(
        Obj::to_rec({{"a", 1}, {"b", 2}})
            ->match(Obj::to_rec({{"a", *Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(-10))}))})},
                                 {*Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::eq(Obj::to_str("c"))}))}), 2}})));
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
    Fluent f = __(*BCode::to_bcode({Insts::plus(o_p(1))})).plus(2).mult(3);
    f.forEach<Obj>([](const Obj_p &x) { LOG(INFO, "%s\n", x->toString().c_str()); });
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_bool); //
      FOS_RUN_TEST(test_int); //
      FOS_RUN_TEST(test_real); //
      FOS_RUN_TEST(test_str); //
      FOS_RUN_TEST(test_lst); //
      FOS_RUN_TEST(test_rec); //
      // FOS_RUN_TEST(test_inst_bcode); //
  )

}; // namespace fhatos

SETUP_AND_LOOP();


#endif
