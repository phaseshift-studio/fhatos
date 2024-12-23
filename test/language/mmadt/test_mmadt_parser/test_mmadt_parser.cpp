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

#ifndef mmadt_test_mmadt_parser_cpp
#define mmadt_test_mmadt_parser_cpp
#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /abc/#
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_comment_parsing() {
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("_oO a single line comment Oo_"));
    //FOS_TEST_ERROR("_oO\nsdfOo_asdf Oo_");
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("_oO sdfdasdf\n\t\nsdfsfasfsf Oo_"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("_oO start Oo_\nint[1]\n_oO end Oo_"));
    FOS_TEST_OBJ_EQUAL(str("abc"), PROCESS("\n\n_oO \n\nstart Oo_\n\nstr['abc']\n\n_oO a comment_oO\n_oO end\n\n Oo_"));
  }

  void test_type_parsing() {
    TEST_ASSERT_EQUAL(OType::NOOBJ, PROCESS("noobj")->o_type());
  }

  void test_noobj_parsing() {
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("noobj"));
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("noobj[noobj]"));
    // FOS_TEST_OBJ_EQUAL(vri(NOOBJ_FURI), PROCESS("noobj.type()"));
  }

  void test_bool_parsing() {
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("<bool>[false]"));
    FOS_TEST_OBJ_NOT_EQUAL(dool(false), PROCESS("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("bool[start(false)]"));
  }

  void test_int_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("1"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), PROCESS("-10"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), PROCESS("   -10   "));
    FOS_TEST_OBJ_EQUAL(jnt(-10,INT_FURI,id_p("/abc/ten")), PROCESS("   -10@/abc/ten   "));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("int[0]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50), PROCESS("int?int<=int[-50]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50,INT_FURI,id_p("/abc/x/y/z")), PROCESS("int?int<=int[-50]@</abc/x/y/z>"));
    FOS_TEST_OBJ_EQUAL(jnt(-50,INT_FURI,id_p("/abc/x/y/z")), PROCESS("<int?int<=int>[-50]@/abc/x/y/z"));
    FOS_TEST_OBJ_EQUAL(jnt(-50,INT_FURI,id_p("/abc/x/y/z")), PROCESS("<int?int<=int>[-50]@</abc/x/y/z>"));
  }

  void test_real_parsing() {
    FOS_TEST_OBJ_EQUAL(real(1.0), PROCESS("1.0"));
    FOS_TEST_OBJ_EQUAL(real(-10.0), PROCESS("-10.0"));
    FOS_TEST_OBJ_EQUAL(real(-10.01), PROCESS("-10.01"));
    FOS_TEST_OBJ_EQUAL(real(-10.01), PROCESS("-10.01    "));
    FOS_TEST_OBJ_EQUAL(real(0), PROCESS("real[0.0]"));
    FOS_TEST_OBJ_EQUAL(real(0), PROCESS("real  [  0.0  ]"));
    FOS_TEST_OBJ_NOT_EQUAL(real(0), PROCESS("int[0]"));
  }

  void test_str_parsing() {
    FOS_TEST_OBJ_EQUAL(str("one.oh"), PROCESS("'one.oh'"));
    FOS_TEST_OBJ_EQUAL(str("negatIVE te  N"), PROCESS("'negatIVE te  N'"));
    FOS_TEST_OBJ_EQUAL(str(""), PROCESS("str['']"));
    FOS_TEST_OBJ_EQUAL(str("abc"), PROCESS("str['abc']"));
    TEST_ASSERT_EQUAL_STRING("b\\'c", PROCESS("'a\\''.'b\\'c'")->str_value().c_str());
    TEST_ASSERT_EQUAL_STRING("a\"b\"c", PROCESS("'a\"b\"c'")->str_value().c_str());
    TEST_ASSERT_EQUAL_STRING("a\\'b\\'c", PROCESS("'a\\'b\\'c'")->str_value().c_str());
  }

  void test_uri_parsing() {
    FOS_TEST_OBJ_EQUAL(vri("http://www.fhatos.org"), PROCESS("<http://www.fhatos.org>"));
    FOS_TEST_OBJ_EQUAL(vri(""), PROCESS("<>"));
    FOS_TEST_OBJ_EQUAL(vri(":"), PROCESS("<:>"));
    FOS_TEST_OBJ_EQUAL(vri("a"), PROCESS("a"));
    FOS_TEST_OBJ_EQUAL(vri("../../a"), PROCESS("<../../a>"));
    FOS_TEST_OBJ_EQUAL(vri("abc/cba"), PROCESS("abc/cba"));
    FOS_TEST_OBJ_EQUAL(vri("aBc_cBa"), PROCESS("aBc_cBa"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), PROCESS("uri[aaa_bbb/ccc/`.`./ddd]"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), PROCESS("uri[<aaa_bbb/ccc/../ddd>]"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), PROCESS("<aaa_bbb/ccc/../ddd>"));
    FOS_TEST_OBJ_NTEQL(vri("aaa_bbb/ccc/../ddd"), PROCESS("aaa_bbb/ccc/`./ddd"));
  }

  void test_lst_parsing() {
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("[]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), PROCESS("[1,2,3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), PROCESS("[1   , 2 , 3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),lst({jnt(2),jnt(4)}),jnt(3)}), PROCESS("[1,[2,4],3]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("['a',['b',['e','c']],'d']"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("lst[['a',lst[['b',lst[['e','c']]]],'d']]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("lst[['a',['b',lst[['e','c']]],'d']]"));
  }

  void test_rec_parsing() {
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS("[=>]"));
    TEST_ASSERT_EQUAL_STRING("[=>]", rec()->toString(NO_ANSI_PRINTER).c_str());
    FOS_TEST_OBJ_NTEQL(rec(), PROCESS("[]"));
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS("rec[[=>]]"));
    FOS_TEST_OBJ_NTEQL(rec(), PROCESS("[a=>1]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),str("b")},{jnt(3),str("c")}}),
                       PROCESS("[1=>'a',2=>'b',3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),str("b")},{jnt(3),str("c")}}),
                       PROCESS("[  1  => 'a', 2=>  'b',3  =>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec()},{jnt(3),str("c")}}),
                       PROCESS("[1=>'a',2=>[=>],3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       PROCESS("[1=>'a',2=>['b'=>['d'=>4]],3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       PROCESS("rec[[1=>'a',2=>rec[['b'=>rec[['d'=>4]]]],3=>'c']]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       PROCESS("rec[[1=>'a',2=>['b'=>rec[['d'=>4]]],3=>'c']]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       PROCESS("rec[[1=>a,2=>[b=>[d=>4]],3=>c]]"));
//    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
 //                      PROCESS("rec\t  [\t[1 =>  a,2  =>[\t\t\tb =>[d => 4] ], 3=>   c]  ]"));
    FOS_TEST_OBJ_NTEQL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       PROCESS("rec[[2=>[b=>[d=>4]],3=>c]]")); // no 1=>a
  }

  void test_inst_parsing() {
    // TODO: TEST_ASSERT_EQUAL_STRING("play?play<=int[plus(2)]",
    //                  PROCESS("|play?play<=int[plus(2)]")->toString(SERIALIZER_PRINTER).
    //                c_str());
FOS_TEST_OBJ_EQUAL(dool(false),PROCESS("|bcode?bool<=bool|(a=>_)[is(eq(*a))]@/abc/bool/true_static;.start({false})./abc/bool/true_static()"));
    FOS_TEST_OBJ_EQUAL(dool(false),PROCESS(""
                                           "|bcode?bool<=noobj|(a=>_)[is(eq(*a))]@/abc/bool/true_static;."
                                           "/abc/bool/true_static(false)"));
  //  FOS_TEST_OBJ_EQUAL(dool(false),PROCESS(""
       //                                    "|bcode?bool<=noobj|(_)[is(eq(*_0))]@/abc/bool/true_static;."
       //                                    "/abc/bool/true_static(false)"));
    // /"<nat?nat<=int>|(x=>_,y=>2)[is(gt(0))]"
  }

  void test_inst_sugar_parsing() {
    ////////// start{}
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("2.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("{2}.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("{2,noobj,noobj}.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("{0,1,2}.plus(1).is(gt(2))"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("{0,1,2}.plus(1).count()"));
    ////////// x.y.z
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("'a'.'b'.'c'.4"));
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("<a.b.c>.4"));
  //  FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("1.(2).3.(4)"));
  //  FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("0.(1).plus((3))"));
  //  FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("(4)"));
    // FOS_TEST_OBJ_EQUAL(real(3.4), PROCESS("(1.2).(3.4)"));
    ////////// x
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3 x 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(6), PROCESS("3x2")->apply());
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3x 2"));
    // FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("  3x 2  "));
    //FOS_TEST_OBJ_NTEQL(jnt(6), PROCESS("3 x2")->apply());
    ////////// +
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3 + 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(5), PROCESS("3+2")->apply());
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3+ 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(5), PROCESS("3 +2")->apply());
    //FOS_TEST_OBJ_NTEQL(jnt(5), PROCESS("  start(3) + 2 x 2 .plus(-5) "));
    ////////// proto.map
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("9.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("{9}.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("start({9}).plus(1)"));
    FOS_TEST_ERROR("map(9).plus(1)");
    ////////// -< >-
    FOS_TEST_OBJ_EQUAL(lst({vri("a"),jnt(1),vri("a/b")}), PROCESS("a-<[_,1,+ /b]"))
    FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[+ b/]>-x c"))
    // TODO:     FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[_ + b/]>-x c"))
    FOS_TEST_OBJ_EQUAL(str("abc"), PROCESS("'a' + 'b' + 'c'"))
    ////////// _/x\_
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("[1]_/ x 3\\__/+ 5\\_>-"))
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("1-<[+ 2]_/ + 5\\_>-"))
    ////////// |
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("start(1).8"))
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("1|8"))
    FOS_TEST_OBJ_EQUAL(BCODE_FURI, PROCESS("1|plus(7)")->tid());
    ////////// ==
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4),jnt(6)}), PROCESS("[1,2,3]==[x 2,mult(2),x 2]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4),jnt(6)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[_,_]"));
    FOS_TEST_OBJ_NTEQL(lst({jnt(2),jnt(4)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[+ 0,plus(1)]"));
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[]"));
    ////////// *
    FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("/abc/o1o -> 101"));
    FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("*/abc/o1o"));
    FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("*(/abc/o1o)"));
    FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("*(/abc/o12,101)"));
    // FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("</abc/o1o>.*()"));
    FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("</abc/o1o>.*(_)"));
    // FOS_TEST_OBJ_EQUAL(jnt(101),PROCESS("</abc/o1o>*"));
  }

  void test_apply_mono_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.6"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.map(6)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(3)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(_)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(mult(1))"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.  plus( mult ( 1)   )"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3     .plus(  mult( 1  )   )"));
  }

  void test_apply_poly_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("[a=>1,b=>7].a"));
    FOS_TEST_OBJ_EQUAL(jnt(2), PROCESS("[a=>7,b=>[c=>2]].b/c"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("[7,3,5].<1>"));
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("[8,7,4].<2>"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("[8,[a=>[b=>5],c=>7]].<1>.a.b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[8,[a=>[b=>6],c=>7]].<1>.a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(7), PROCESS("[  8  ,[   a=>[ b => 7],c   =>   7]   ].<1>.a/b"));
//    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("/abc -> [8,[a=>[b=>8],c=>7]];.*</abc/1/a/b>"));
//    FOS_TEST_OBJ_EQUAL(jnt(9), PROCESS("/abc -> [8,[a=>[b=>9],c=>7]]; */abc/1/a/b"));
//    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("/abc -> [8,[a=>[b=>6],c=>7]]; */abc/1/a.b.to(/abc/x); */abc/x + -6"));
  }

  void test_type_definition_parsing() {
    const ID_p nat = id_p("/abc/nat");
    // FOS_TEST_OBJ_EQUAL(jnt(5,nat), PROCESS("(/abc/nat -> |/abc/nat?/abc/nat<=int[is(gt(0))]); /abc/nat[5]"));
  }

  FOS_RUN_TESTS( //
    //FOS_RUN_TEST(test_comment_parsing); //
    FOS_RUN_TEST(test_noobj_parsing); //
    FOS_RUN_TEST(test_type_parsing); //
    FOS_RUN_TEST(test_bool_parsing); //
    FOS_RUN_TEST(test_int_parsing); //
    FOS_RUN_TEST(test_real_parsing); //
    FOS_RUN_TEST(test_str_parsing); //
    FOS_RUN_TEST(test_uri_parsing); //
    FOS_RUN_TEST(test_lst_parsing); //
    FOS_RUN_TEST(test_rec_parsing); //
 // FOS_RUN_TEST(test_inst_parsing); //
    //////////////////////////////////
   FOS_RUN_TEST(test_inst_sugar_parsing); //
    FOS_RUN_TEST(test_apply_mono_parsing); //
    FOS_RUN_TEST(test_apply_poly_parsing); //
    FOS_RUN_TEST(test_type_definition_parsing); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
