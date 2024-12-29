/*******************************************************************************
  FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_test_obj_hpp
#define fhatos_test_obj_hpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_SHARED_MEMORY
#include <../test/test_fhatos.hpp>
#include <util/obj_helper.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

 /* void test_bool() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "bool/truth"), Obj::to_bcode()); //
    const Bool_p bool_a = share(Bool(true, FOS_TYPE_PREFIX "bool/truth"));
    const Bool_p bool_b = share(Bool(false, "truth"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*bool_a).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(ID(FOS_TYPE_PREFIX "bool/truth"), *bool_a->tid());
    FOS_TEST_ASSERT_EQUAL_FURI(ID(FOS_TYPE_PREFIX "bool/truth"), *bool_b->tid());
    FOS_TEST_OBJ_NOT_EQUAL(bool_a, bool_b);
    FOS_TEST_OBJ_NOT_EQUAL(bool_b, bool_a);
    FOS_TEST_OBJ_EQUAL(bool_a, bool_a);
    FOS_TEST_OBJ_EQUAL(bool_b, bool_b);
    FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false, FOS_TYPE_PREFIX "bool/truth")), share(Bool(*bool_a && *bool_b)));
    FOS_TEST_OBJ_EQUAL(share(Bool(false)), share(Bool(*bool_a && *bool_b)));
    TEST_ASSERT_TRUE(bool_a->bool_value());
    TEST_ASSERT_FALSE(bool_a->is_bcode());
    TEST_ASSERT_FALSE(bool_a->is_noobj());
    TEST_ASSERT_EQUAL(OType::BOOL, bool_a->o_type());
    FOS_TEST_OBJ_EQUAL(bool_a, bool_a->apply(bool_b));
    ///
    TEST_ASSERT_TRUE(bool_a->match(bool_a));
    TEST_ASSERT_TRUE(Obj::to_bool(true)->match(Obj::to_bcode({Insts::is(Obj::to_bcode())})));
    TEST_ASSERT_FALSE(Obj::to_bool(false)->match(Obj::to_bcode({Insts::is(Obj::to_bcode())})));
    /* const Bool_p boolBCode = share(Bool(__().gt(5)._bcode->bcode_value(), "/_bcode/secret"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(boolBCode).c_str());
     FOS_TEST_OBJ_NOT_EQUAL(share(Bool(false)), boolBCode->apply(share(Int(4))));
   //  FOS_TEST_OBJ_EQUAL(share(Bool(false, "/bool/secret")), boolBCode->apply(share(Int(3,int_t("nat")))));
     FOS_TEST_ASSERT_EXCEPTION(__().lt(2)._bcode->apply(share(Bool(true, "/bool/truth"))));*/
  //}

  /*void test_int() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "int/age"), Obj::to_bcode()); //
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "int/nat"), Obj::to_bcode()); //
    const Int_p intA = share(Int(1));
    const Int_p intB = share<Int>(Int(1));
    const Int_p intC = share(Int(1, FOS_TYPE_PREFIX "int/age"));
    const Int_p intD = ptr<Int>(new Int(2, "nat"));
    ///
    const Int_p int5 = share(Int(5, "age"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*int5).c_str());

    TEST_ASSERT_FALSE(intA->is_bcode());
    TEST_ASSERT_EQUAL_STRING(FOS_TYPE_PREFIX "int/", intA->tid()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("int", intA->tid()->name().c_str());
    TEST_ASSERT_EQUAL_STRING("/type", intA->tid()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("int/", intA->tid()->path(1, 2).c_str());
    TEST_ASSERT_EQUAL(OType::INT, intA->o_type());
    TEST_ASSERT_FALSE(intA->is_noobj());
    TEST_ASSERT_TRUE(intA->match(intB));
    TEST_ASSERT_FALSE(intA->match(jnt(987)));
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*intC->tid(), *intA->as(id_p("age"))->tid());
    FOS_TEST_OBJ_EQUAL(intA, intB);
    FOS_TEST_OBJ_EQUAL(intB, intA);
    FOS_TEST_OBJ_NOT_EQUAL(intB, intB->as(FOS_TYPE_PREFIX "int/age"));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age"));
    FOS_TEST_OBJ_EQUAL(intC, intB->as(FOS_TYPE_PREFIX "int/age"));
    FOS_TEST_OBJ_EQUAL(intA, intC->as(FOS_TYPE_PREFIX "int/"));
    FOS_TEST_OBJ_EQUAL(intA, intA->as(FOS_TYPE_PREFIX "int/"));
    /// apply
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intB));
    FOS_TEST_OBJ_EQUAL(intA, intA->apply(intA));
    FOS_TEST_OBJ_EQUAL(intA->as("age"), intA->as(FOS_TYPE_PREFIX "int/age")->apply(intB));
    FOS_TEST_OBJ_EQUAL(intC, intA->as("age")->apply(intB));
    /// relations
    TEST_ASSERT_GREATER_THAN_INT(intA->as("nat")->int_value(), intD->int_value());
    TEST_ASSERT_LESS_THAN_INT(intD->int_value(), intB->as(FOS_TYPE_PREFIX "int/nat")->int_value());
    /// match
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_int(22)));
    TEST_ASSERT_TRUE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("nat")));
    TEST_ASSERT_FALSE(Obj::to_int(22)->as("nat")->match(Obj::to_int(22)->as("age")));
    TEST_ASSERT_TRUE(Obj::to_int(22)->match(Obj::to_bcode({
      Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(0))}))
      })));
    TEST_ASSERT_FALSE(
        Obj::to_int(22)->match(Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(23))}))})));
    ////// BYTECODE
    /* const Int_p intBCode = share(Int(__().plus(Int(0, "/int/age"))._bcode->bcode_value(), "/_bcode/age"));
     FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(intBCode).c_str());
     // TEST_ASSERT_TRUE(intBCode->is_bcode());
     TEST_ASSERT_EQUAL_STRING("/_bcode/age", intBCode->tid()->toString().c_str());
     TEST_ASSERT_EQUAL_STRING("age", intBCode->tid()->lastSegment().c_str());
     /// apply
     FOS_TEST_OBJ_EQUAL(intC, intBCode->apply(intC));
     FOS_TEST_ASSERT_EXCEPTION(intBCode->apply(share<Int>(Int(2, "/nat"))))*/
  //}

  /*void test_real() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/money"), Obj::to_bcode()); //
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/weight"), Obj::to_bcode()); //
    const Real_p realA = share(Real(1.0f));
    const Real_p realB = share<Real>(Real(1.0f));
    const Real_p realC = share(Real(1.0f, FOS_TYPE_PREFIX "real/money"));
    const Real_p realD = ptr<Real>(new Real(2.0f, "weight"));
    ///
    const Real_p real5 = share(Real(5.0f, "money"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*real5).c_str());

    TEST_ASSERT_FALSE(realA->is_bcode());
    TEST_ASSERT_EQUAL_STRING(FOS_TYPE_PREFIX "real/", realA->tid()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("real", realA->tid()->name().c_str());
    TEST_ASSERT_EQUAL_STRING("/type", realA->tid()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("real/", realA->tid()->path(1, 2).c_str());
    TEST_ASSERT_EQUAL(OType::REAL, realA->o_type());
    TEST_ASSERT_FALSE(realA->is_noobj());
    TEST_ASSERT_TRUE(realA->match(realB));
    TEST_ASSERT_FALSE(realA->match(real(987.12)));
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*realC->tid(), *realA->as(id_p("money"))->tid());
    FOS_TEST_OBJ_EQUAL(realA, realB);
    FOS_TEST_OBJ_EQUAL(realB, realA);
    FOS_TEST_OBJ_NOT_EQUAL(realB, realB->as(FOS_TYPE_PREFIX "real/money"));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money"));
    FOS_TEST_OBJ_EQUAL(realC, realB->as(FOS_TYPE_PREFIX "real/money"));
    FOS_TEST_OBJ_EQUAL(realA, realC->as(FOS_TYPE_PREFIX "real/"));
    FOS_TEST_OBJ_EQUAL(realA, realA->as(FOS_TYPE_PREFIX "real/"));
    /// apply
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realB));
    FOS_TEST_OBJ_EQUAL(realA, realA->apply(realA));
    FOS_TEST_OBJ_EQUAL(realA->as("money"), realA->as(FOS_TYPE_PREFIX "real/money")->apply(realB));
    FOS_TEST_OBJ_EQUAL(realC, realA->as("money")->apply(realB));
    /// relations
#ifdef NATIVE
    TEST_ASSERT_GREATER_THAN_FLOAT(realA->as("weight")->real_value(), realD->real_value());
    TEST_ASSERT_LESS_THAN_FLOAT(realD->real_value(), realB->as(FOS_TYPE_PREFIX "real/weight")->real_value());
#endif
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
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "str/first_name"), Obj::to_bcode()); //
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "str/letter"), Obj::to_bcode()); //
    const Str strA = *Obj::to_str("fhat", id_p(FOS_TYPE_PREFIX "str/first_name"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(strA).c_str());
    TEST_ASSERT_FALSE(strA.is_bcode());
    TEST_ASSERT_EQUAL_STRING("fhat", strA.str_value().c_str());
    TEST_ASSERT_EQUAL(OType::STR, strA.o_type());
    TEST_ASSERT_TRUE(strA.match(str("fhat"), false));
    TEST_ASSERT_FALSE(strA.match(str("fhat"), true));
  }

  void test_uri() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "uri/webpage"), Obj::to_bcode()); //
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "uri/ftp"), Obj::to_bcode()); //
    const Uri_p uriA = share(Uri(fURI("home/web/html/index.html")));
    const Uri_p uriB = vri("home/web/html/index.html");
    const Uri_p uriC = share(Uri(fURI("home/web/html/index.html"), FOS_TYPE_PREFIX "uri/webpage"));
    const Uri_p uriD = ptr<Uri>(new Uri(fURI("ftp://localhost:23/"), FOS_TYPE_PREFIX "uri/ftp"));
    ///
    const Uri_p uriE = share(Uri(fURI("http://index.org/index.html"), "webpage"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(*uriE).c_str());

    TEST_ASSERT_FALSE(uriA->is_bcode());
    TEST_ASSERT_EQUAL_STRING(FOS_TYPE_PREFIX "uri/", uriA->tid()->toString().c_str());
    TEST_ASSERT_EQUAL_STRING("uri", uriA->tid()->name().c_str());
    TEST_ASSERT_EQUAL_STRING("/type", uriA->tid()->path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("uri/", uriA->tid()->path(1, 2).c_str());
    TEST_ASSERT_EQUAL(OType::URI, uriA->o_type());
    TEST_ASSERT_FALSE(uriA->is_noobj());
    TEST_ASSERT_TRUE(uriA->match(uriB));
    TEST_ASSERT_FALSE(uriA->match(vri("http://nothing.org")));
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(*uriC->tid(), *uriA->as(id_p("webpage"))->tid());
    FOS_TEST_OBJ_EQUAL(uriA, uriB);
    FOS_TEST_OBJ_EQUAL(uriB, uriA);
    FOS_TEST_OBJ_NOT_EQUAL(uriB, uriB->as(FOS_TYPE_PREFIX "uri/webpage"));
    FOS_TEST_OBJ_EQUAL(uriC, uriA->as("webpage"));
    FOS_TEST_OBJ_EQUAL(uriC, uriB->as(FOS_TYPE_PREFIX "uri/webpage"));
    FOS_TEST_OBJ_EQUAL(uriA, uriC->as(FOS_TYPE_PREFIX "uri/"));
    FOS_TEST_OBJ_EQUAL(uriA, uriA->as(FOS_TYPE_PREFIX "uri/"));
    /// apply
    FOS_TEST_OBJ_EQUAL(uriA, uriA->apply(uriB));
    FOS_TEST_OBJ_EQUAL(uriA, uriA->apply(uriA));
    FOS_TEST_OBJ_EQUAL(uriA->as("webpage"), uriA->as(FOS_TYPE_PREFIX "uri/webpage")->apply(uriB));
    FOS_TEST_OBJ_EQUAL(uriC, uriA->as("webpage")->apply(uriB));
    /// relations // TODO
    // TEST_ASSERT_GREATER_THAN_INT(uriA->as("ftp")->uri_value(), uriD->int_value());
    // TEST_ASSERT_LESS_THAN_INT(uriD->uri_value(), uriB->as(FOS_TYPE_PREFIX "int/nat")->int_value());
    /// match
    TEST_ASSERT_TRUE(vri("example.com")->match(Obj::to_uri("example.com")));
    TEST_ASSERT_TRUE(vri("/fhatos/index.html")->as("webpage")->match(vri("/+/index.html")->as("webpage")));
    TEST_ASSERT_FALSE(vri("/fhatos/index")->as("webpage")->match(vri("/fhatos/index")->as("ftp")));
    TEST_ASSERT_TRUE(vri("/a/b/c")->match(Obj::to_bcode({
      Insts::is(Obj::to_bcode({Insts::eq(vri("/a/b/c"))}))
      })));
    TEST_ASSERT_FALSE(vri("/a/b/c")->match(Obj::to_bcode({
      Insts::is(Obj::to_bcode({Insts::eq(vri("/a/b/c/d"))}))
      })));
  }

  void test_lst() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "lst/ones"), Obj::to_lst({1, 1, 1}));
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
    lstC.lst_set(jnt(0), jnt(1));
    FOS_TEST_OBJ_EQUAL(&lstC, &lstA);
    lstC.lst_set(jnt(4), jnt(5));
    FOS_TEST_OBJ_NOT_EQUAL(&lstC, &lstA);
    for (int i = 0; i < 4; i++) {
      FOS_TEST_OBJ_EQUAL(jnt(i + 1), lstA.lst_get(jnt(i)));
      FOS_TEST_OBJ_EQUAL(jnt(i + 1), lstB.lst_get(jnt(i)));
      FOS_TEST_OBJ_EQUAL(jnt(i + 1), lstC.lst_get(jnt(i)));
    }
    const Lst_p lstD = Obj::to_lst({1, 1, 1});
    TEST_ASSERT_EQUAL_STRING(FOS_TYPE_PREFIX "lst/ones",
                             lstD->as(FOS_TYPE_PREFIX "lst/ones")->tid()->toString().c_str());
    try {
      Obj_p x = lstA.as(FOS_TYPE_PREFIX "lst/ones");
      LOG(ERROR, "%s should have not been castable\n", x->toString().c_str());
      TEST_FAIL_MESSAGE("Should throw exception");
    } catch (const fError &) {
      TEST_ASSERT_TRUE(true);
    }
  }

  void test_rec() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/mail"), Obj::to_bcode()); //
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/cost"), Obj::to_bcode()); //
    const Rec recA = *Obj::to_rec({
        {str("a"), jnt(1)},
        {str("b"), jnt(2)}
    });
    const Rec recB = *Obj::to_rec({
        {str("a"), jnt(1)},
        {str("b"), jnt(2)}
    });
    const Rec recC =
        Obj(share(Obj::RecMap<>({
                make_pair<const Obj_p, Obj_p>(Obj::to_str("a", id_p(FOS_TYPE_PREFIX "str/letter")),
                                              share(Int(1))),
                make_pair<const Obj_p, Obj_p>(Obj::to_str("b", id_p(FOS_TYPE_PREFIX "str/letter")),
                                              share(Int(2)))})),
            OType::REC, id_p(FOS_TYPE_PREFIX "rec/mail"));
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(recC).c_str());
    TEST_ASSERT_TRUE(recA == recB);
    TEST_ASSERT_FALSE(recA != recB);
    TEST_ASSERT_FALSE(recA == recC);
    TEST_ASSERT_TRUE(recA != recC);
    ////
    TEST_ASSERT_EQUAL_INT(1, recA.rec_get(str("a"))->int_value());
    TEST_ASSERT_EQUAL_INT(2, recA.rec_get(str("b"))->int_value());
    TEST_ASSERT_TRUE(recA.rec_get(str("c"))->is_noobj());
    recA.rec_set(str("b"), jnt(12));
    TEST_ASSERT_EQUAL_INT(12, recA.rec_get(str("b"))->int_value());
    recA.rec_set(str("b"), real(202.5f, id_p("cost")));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 202.5f, recA.rec_get(str("b"))->real_value());
    recA.rec_delete(*str("b"));
    TEST_ASSERT_TRUE(recA.rec_get(str("b"))->is_noobj());
    FOS_TEST_MESSAGE("\n%s\n", ObjHelper::objAnalysis(recA).c_str());
    // match
    TEST_ASSERT_TRUE(Obj::to_rec({
      {str("a"), jnt(1)},
      {str("b"), jnt(2)}})->match(Obj::to_rec({
      {str("a"), jnt(1)},
      {str("b"), jnt(2)}})));
    TEST_ASSERT_TRUE(Obj::to_rec({
      {str("a"), jnt(1)},
      {str("b"), jnt(2)},
{str("c"), jnt(3)}
      })->match(Obj::to_rec({{str("a"),  jnt(1)},{str("b"),  jnt(2)}})));
    TEST_ASSERT_TRUE(
        Obj::to_rec({
          {str("a"),  jnt(1)},
          {str("b"), jnt(2)}})->match(Obj::to_rec({
          {str("a"), Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(0))}))})},
          {str("b"), jnt(2)}})));
    TEST_ASSERT_FALSE(
        Obj::to_rec({{str("a"), jnt(1)},{str("b"), jnt(2)}})->match(Obj::to_rec({
          {str("a"), Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(3))}))})},
          {str("b"), jnt(2)}})));
    TEST_ASSERT_TRUE(
        Obj::to_rec({{str("a"), jnt(1)},{str("b"),jnt(2)}})->match(Obj::to_rec({
          {str("a"), Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(Obj::to_int(-10))}))})},
          { Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::eq(Obj::to_str("b"))}))}), jnt(2)}})));
    TEST_ASSERT_FALSE(
        Obj::to_rec({{str("a"), jnt(1)},{str("b"), jnt(2)}})->match(Obj::to_rec({
          {str("a"), Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::gt(
            Obj::to_int(-10))}))})},
          { Obj::to_bcode({Insts::is(Obj::to_bcode({Insts::eq(Obj::to_str("c"))}))}), jnt(2)}})));
  }*/

void test_inst() {
  const Inst_p i1 = Obj::create(InstValue(Obj::to_rec({{"a",jnt(10)}}),
                                           make_shared<InstF>(Obj::to_bcode()),
                                           IType::ONE_TO_ONE,noobj()),OType::INST,id_p("myinst"));
  const Inst_p i2 = Obj::create(InstValue(Obj::to_rec({{"a",jnt(10)}}),
                                           make_shared<InstF>(make_shared<Cpp>([](const Obj_p& lhs, const InstArgs&) -> Obj_p {return lhs;})),
                                           IType::ONE_TO_ONE,noobj()),OType::INST,id_p("myinst"));
  FOS_TEST_OBJ_NOT_EQUAL(i1,i2);
  FOS_TEST_ASSERT_EQUAL_FURI(*i1->tid(),*i2->tid());
  FOS_TEST_OBJ_EQUAL(i1->inst_args(),i2->inst_args());
  TEST_ASSERT_EQUAL(i1->itype(),i2->itype());
  TEST_ASSERT_NOT_EQUAL_INT(i1->toString(NO_ANSI_PRINTER).find("_"),string::npos);
  TEST_ASSERT_EQUAL_INT(i2->toString(NO_ANSI_PRINTER).find("_"),string::npos);
}

  void test_serialization() {
    const List<Obj_p> objs = {
        Obj::to_noobj(),
        // TODO: Obj::create(Any(),OType::OBJ,INT_FURI),
        Obj::to_int(1),
        Obj::to_int(-453),
        Obj::to_real(12.035f),
        Obj::to_str("fhatos"),
        Obj::to_uri("aaaa"),
        //Obj::to_uri("<abba>"),
        Obj::to_lst({jnt(1), jnt(7), str("abc"), vri("hello/fhat/aus")}),
        Obj::to_rec({{"a", jnt(2)}, {"b", jnt(3)}})
       // PROCESS("parsed_inst(a=>10,b=>20)[plus(*a).plus(*b)]")->none_one_all(),
       //Obj::create(InstValue(Obj::to_rec({{"a",jnt(10)}}),
        //                                 make_shared<InstF>(Obj::to_bcode()),
         //                                IType::ONE_TO_ONE,noobj()),OType::INST,id_p("myinst"))
    };
    for (const auto &obj_a: objs) {
      const BObj_p bobj = obj_a->serialize();
      const Obj_p obj_b = Obj::deserialize(bobj);
      FOS_TEST_OBJ_EQUAL(obj_a, obj_b);
    }
  }

  FOS_RUN_TESTS( //
    //  FOS_RUN_TEST(test_bool); //
    //  FOS_RUN_TEST(test_int); //
    //  FOS_RUN_TEST(test_real); //
    //  FOS_RUN_TEST(test_str); //
     // FOS_RUN_TEST(test_uri); //
     // FOS_RUN_TEST(test_lst); //
     // FOS_RUN_TEST(test_rec); //
      FOS_RUN_TEST(test_inst); //
      FOS_RUN_TEST(test_serialization); //
      )
}; // namespace fhatos

SETUP_AND_LOOP();


#endif