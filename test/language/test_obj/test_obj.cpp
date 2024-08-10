#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#define FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>
#include <util/obj_helper.hpp>
namespace fhatos {

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_bool() {
    Types::singleton()->saveType(id_p("/type/bool/truth"), Obj::to_bcode()); //
    const Bool_p boolA = share(Bool(true, "/type/bool/truth"));
    const Bool_p boolB = share(Bool(false, "truth"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*boolA).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/type/bool/truth"), *boolA->id());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/type/bool/truth"), *boolB->id());
    FOS_TEST_OBJ_NOT_EQUAL(boolA, boolB);
    FOS_TEST_OBJ_NOT_EQUAL(boolB, boolA);
    FOS_TEST_OBJ_EQUAL(boolA, boolA);
    FOS_TEST_OBJ_EQUAL(boolB, boolB);
    FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false, "/type/bool/truth")), share(Bool(*boolA && *boolB)));
    FOS_TEST_OBJ_EQUAL(share(Bool(false)), share(Bool(*boolA && *boolB)));
    TEST_ASSERT_TRUE(boolA->bool_value());
    TEST_ASSERT_FALSE(boolA->isBytecode());
    TEST_ASSERT_FALSE(boolA->isNoObj());
    TEST_ASSERT_EQUAL(OType::BOOL, boolA->o_type());
    FOS_TEST_OBJ_EQUAL(boolA, boolA->apply(boolB));
    ///
    TEST_ASSERT_TRUE(boolA->match(boolA));
    TEST_ASSERT_TRUE(Obj::to_bool(true)->match(Obj::to_bcode({Insts::is(Obj::to_bcode())})));
    TEST_ASSERT_FALSE(Obj::to_bool(false)->match(Obj::to_bcode({Insts::is(Obj::to_bcode())})));
    /* const Bool_p boolBCode = share(Bool(__().gt(5)._bcode->bcode_value(), "/_bcode/secret"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolBCode).c_str());
     FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false)), boolBCode->apply(share(Int(4))));
   //  FOS_TEST_OBJ_EQUAL(share(Bool(false, "/bool/secret")), boolBCode->apply(share(Int(3,int_t("nat")))));
     FOS_TEST_ASSERT_EXCEPTION(__().lt(2)._bcode->apply(share(Bool(true, "/bool/truth"))));*/
  }

  void test_int() {
    Types::singleton()->saveType(id_p("/type/int/age"), Obj::to_bcode()); //
    Types::singleton()->saveType(id_p("/type/int/nat"), Obj::to_bcode()); //
    const Int_p intA = share(Int(1));
    const Int_p intB = share<Int>(Int(1));
    const Int_p intC = share(Int(1, "/type/int/age"));
    const Int_p intD = ptr<Int>(new Int(2, "nat"));
    ///
    const Int_p int5 = share(Int(5, "age"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*int5).c_str());

    TEST_ASSERT_FALSE(intA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/type/int/", intA->id()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("int", intA->id()->name());
    TEST_ASSERT_EQUAL_STRING("/type", intA->id()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("int/", intA->id()->path(1, 2).c_str());
    TEST_ASSERT_EQUAL(OType::INT, intA->o_type());
    TEST_ASSERT_FALSE(intA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*intC->id(), *intA->as(id_p("age"))->id());
    FOS_TEST_OBJ_EQUAL(intA, intB);
    FOS_TEST_OBJ_EQUAL(intB, intA);
    FOS_TEST_OBJ_NOT_EQUAL(intB, intB->as("/type/int/age"));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intB->as("/type/int/age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/type/int/age"), intB->split(10, id_p("/type/int/age")));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/type/int/age"), intB->split(10, "age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "/type/int/age"), intB->split(10, "/type/int/age"));
    FOS_TEST_OBJ_EQUAL(intA->split(10, "age"), intB->split(10, "age"));
    FOS_TEST_OBJ_EQUAL(intA, intC->as("/type/int/"));
    FOS_TEST_OBJ_EQUAL(intA, intA->as("/type/int/"));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("age"), intC->split(2));
    FOS_TEST_OBJ_EQUAL(intA->split(2)->as("/type/int/age"), intB->split(10)->split(2)->as("age"));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(2));
    FOS_TEST_OBJ_NOT_EQUAL(intA->split(2)->as("age"), intB->split(3)->as("/type/int/age"));
    /// apply
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intB));
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intA));
    FOS_TEST_OBJ_EQUAL(intA->as("age"), intA->as("/type/int/age")->apply(intB));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age")->apply(intB));
    /// relations
    TEST_ASSERT_GREATER_THAN_INT(intA->as("nat")->int_value(), intD->int_value());
    TEST_ASSERT_LESS_THAN_INT(intD->int_value(), intB->as("/type/int/nat")->int_value());
    /// match
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_int(22)));
    TEST_ASSERT_TRUE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("nat")));
    TEST_ASSERT_FALSE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("age")));
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(0))}))})));
    TEST_ASSERT_FALSE(Obj::to_int(22)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(23))}))})));
    ////// BYTECODE
    /* const Int_p intBCode = share(Int(__().plus(Int(0, "/int/age"))._bcode->bcode_value(), "/_bcode/age"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(intBCode).c_str());
     // TEST_ASSERT_TRUE(intBCode->isBytecode());
     TEST_ASSERT_EQUAL_STRING("/_bcode/age", intBCode->id()->toString().c_str());
     TEST_ASSERT_EQUAL_STRING("age", intBCode->id()->lastSegment().c_str());
     /// apply
     FOS_TEST_OBJ_EQUAL(intC, intBCode->apply(intC));
     FOS_TEST_ASSERT_EXCEPTION(intBCode->apply(share<Int>(Int(2, "/nat"))))*/
  }

  void test_real() {
    Types::singleton()->saveType(id_p("/type/real/money"), Obj::to_bcode()); //
    Types::singleton()->saveType(id_p("/type/real/weight"), Obj::to_bcode()); //
    const Real_p realA = share(Real(1.0f));
    const Real_p realB = share<Real>(Real(1.0f));
    const Real_p realC = share(Real(1.0f, "/type/real/money"));
    const Real_p realD = ptr<Real>(new Real(2.0f, "weight"));
    ///
    const Real_p real5 = share(Real(5.0f, "money"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*real5).c_str());

    TEST_ASSERT_FALSE(realA->isBytecode());
    TEST_ASSERT_EQUAL_STRING("/type/real/", realA->id()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("real", realA->id()->name());
    TEST_ASSERT_EQUAL_STRING("/type", realA->id()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("real/", realA->id()->path(1, 2).c_str());
    TEST_ASSERT_EQUAL(OType::REAL, realA->o_type());
    TEST_ASSERT_FALSE(realA->isNoObj());
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*realC->id(), *realA->as(id_p("money"))->id());
    FOS_TEST_OBJ_EQUAL(realA, realB);
    FOS_TEST_OBJ_EQUAL(realB, realA);
    FOS_TEST_OBJ_NOT_EQUAL(realB, realB->as("/type/real/money"));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money"));
    FOS_TEST_OBJ_EQUAL(realC, realB->as("/type/real/money"));
    FOS_TEST_OBJ_EQUAL(realA->split(10.0f, "/type/real/money"), realB->split(10.0f, "money"));
    FOS_TEST_OBJ_EQUAL(realA, realC->as("/type/real/"));
    FOS_TEST_OBJ_EQUAL(realA, realA->as("/type/real/"));
    FOS_TEST_OBJ_EQUAL(realA->split(2.0f)->as("money"), realC->split(2.0f));
    FOS_TEST_OBJ_EQUAL(realA->split(2.0f)->as("/type/real/money"), realB->split(10.0f)->split(2.0f)->as("money"));
    FOS_TEST_OBJ_NOT_EQUAL(realA->split(2.0f)->as("money"), realB->split(2.0f));
    FOS_TEST_OBJ_NOT_EQUAL(realA->split(2.0f)->as("money"), realB->split(3.0f)->as("/type/real/money"));
    /// apply
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realB));
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realA));
    FOS_TEST_OBJ_EQUAL(realA->as("money"), realA->as("/type/real/money")->apply(realB));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money")->apply(realB));
    /// relations
    TEST_ASSERT_GREATER_THAN_FLOAT(realA->as("weight")->real_value(), realD->real_value());
    TEST_ASSERT_LESS_THAN_FLOAT(realD->real_value(), realB->as("/type/real/weight")->real_value());
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
    Types::singleton()->saveType(id_p("/type/str/first_name"), Obj::to_bcode()); //
    Types::singleton()->saveType(id_p("/type/str/letter"), Obj::to_bcode()); //
    const Str strA = *Obj::to_str("fhat", id_p("/type/str/first_name"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(strA).c_str());
    TEST_ASSERT_FALSE(strA.isBytecode());
    TEST_ASSERT_EQUAL_STRING("fhat", strA.str_value().c_str());
    TEST_ASSERT_EQUAL(OType::STR, strA.o_type());
  }

  void test_lst() {
    Types::singleton()->saveType(id_p("/type/lst/ones"), Obj::to_lst({1, 1, 1}));
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
    lstC.lst_set(obj(0), obj(1));
    FOS_TEST_OBJ_EQUAL(&lstC, &lstA);
    lstC.lst_set(obj(4), obj(5));
    FOS_TEST_OBJ_NOT_EQUAL(&lstC, &lstA);
    for (int i = 0; i < 4; i++) {
      FOS_TEST_OBJ_EQUAL(obj(i + 1), lstA.lst_get(obj(i)));
      FOS_TEST_OBJ_EQUAL(obj(i + 1), lstB.lst_get(obj(i)));
      FOS_TEST_OBJ_EQUAL(obj(i + 1), lstC.lst_get(obj(i)));
    }
    const Lst_p lstD = Obj::to_lst({1, 1, 1});
    TEST_ASSERT_EQUAL_STRING("/type/lst/ones", lstD->as("/type/lst/ones")->id()->toString().c_str());
    try {
      const Obj_p x = lstA.as("/type/lst/ones");
      LOG(ERROR, "%s should have not been castable\n", x->toString().c_str());
      TEST_FAIL_MESSAGE("Should throw exception");
    } catch (const fError &) {
      TEST_ASSERT_TRUE(true);
    }
  }

  void test_rec() {
    Types::singleton()->saveType(id_p("/type/rec/mail"), Obj::to_bcode()); //
    Types::singleton()->saveType(id_p("/type/real/cost"), Obj::to_bcode()); //
    const Rec recA = *Obj::to_rec({{"a", 1}, {"b", 2}});
    const Rec recB = *Obj::to_rec({{"a", 1}, {"b", 2}});
    const Rec recC =
        Obj(share(Obj::RecMap<>(
                {make_pair<const Obj_p, Obj_p>(Obj::to_str("a", id_p("/type/str/letter")), share(Int(1))),
                 make_pair<const Obj_p, Obj_p>(Obj::to_str("b", id_p("/type/str/letter")), share(Int(2)))})),
            id_p("/type/rec/mail"));
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

  void test_serialization() {
    const List<Obj_p> objs = {Obj::to_int(1), Obj::to_int(-453), Obj::to_real(12.035f),
                              // Obj::to_str("fhatos"),
                              // Obj::to_uri("aaaa"),
                              // Obj::to_lst({1, 7, "abc", u("hello/fhat/aus")}),
                              // Obj::to_rec({{u("a"), 2}, {u("b"), 3}}),
                              Obj::to_noobj()};
    for (const auto &objA: objs) {
      const BObj_p bobj = objA->serialize();
      const Obj_p objB = Obj::deserialize<Obj>(bobj);
      FOS_TEST_OBJ_EQUAL(objA, objB);
    }
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_bool); //
      FOS_RUN_TEST(test_int); //
      FOS_RUN_TEST(test_real); //
      FOS_RUN_TEST(test_str); //
      FOS_RUN_TEST(test_lst); //
      FOS_RUN_TEST(test_rec); //
      FOS_RUN_TEST(test_serialization); //
  )
}; // namespace fhatos

SETUP_AND_LOOP();


#endif
