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

  void test_type_parsing() {
    TEST_ASSERT_EQUAL(OType::NOOBJ, OBJ_PARSER("noobj")->o_type());
  }

  void test_noobj_parsing() {
    FOS_TEST_OBJ_EQUAL(noobj(), OBJ_PARSER("noobj"));
  }

  void test_bool_parsing() {
    FOS_TEST_OBJ_EQUAL(dool(true), OBJ_PARSER("true"));
    FOS_TEST_OBJ_EQUAL(dool(false), OBJ_PARSER("false"));
    FOS_TEST_OBJ_EQUAL(dool(true), OBJ_PARSER("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), OBJ_PARSER("bool[false]"));
    FOS_TEST_OBJ_NOT_EQUAL(dool(false), OBJ_PARSER("bool[true]"));
    TEST_ASSERT_NOT_EQUAL(dool(false)->toString(), OBJ_PARSER("bool[map(false)]")->toString());
  }

  void test_int_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(1), OBJ_PARSER("1"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), OBJ_PARSER("-10"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), OBJ_PARSER("   -10   "));
    FOS_TEST_OBJ_EQUAL(jnt(0), OBJ_PARSER("int[0]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50), OBJ_PARSER("int?int<=int[-50]"));
  }

  void test_real_parsing() {
    FOS_TEST_OBJ_EQUAL(real(1.0), OBJ_PARSER("1.0"));
    FOS_TEST_OBJ_EQUAL(real(-10.0), OBJ_PARSER("-10.0"));
    FOS_TEST_OBJ_EQUAL(real(-10.01), OBJ_PARSER("-10.01"));
    FOS_TEST_OBJ_EQUAL(real(-10.01), OBJ_PARSER("-10.01    "));
    FOS_TEST_OBJ_EQUAL(real(0), OBJ_PARSER("real[0.0]"));
    FOS_TEST_OBJ_EQUAL(real(0), OBJ_PARSER("real  [  0.0  ]"));
    FOS_TEST_OBJ_NOT_EQUAL(real(0), OBJ_PARSER("int[0]"));
  }

  void test_str_parsing() {
    FOS_TEST_OBJ_EQUAL(str("one.oh"), OBJ_PARSER("'one.oh'"));
    FOS_TEST_OBJ_EQUAL(str("negatIVE te  N"), OBJ_PARSER("'negatIVE te  N'"));
    FOS_TEST_OBJ_EQUAL(str(""), OBJ_PARSER("str['']"));
    FOS_TEST_OBJ_EQUAL(str("abc"), OBJ_PARSER("str['abc']"));
    TEST_ASSERT_EQUAL_STRING("b\\'c", OBJ_PARSER("'a\\''.'b\\'c'")->apply()->str_value().c_str());
    TEST_ASSERT_EQUAL_STRING("a\"b\"c", OBJ_PARSER("'a\"b\"c'")->str_value().c_str());
    TEST_ASSERT_EQUAL_STRING("a\\'b\\'c", OBJ_PARSER("'a\\'b\\'c'")->str_value().c_str());
  }

  void test_uri_parsing() {
    FOS_TEST_OBJ_EQUAL(vri("http://www.fhatos.org"), OBJ_PARSER("<http://www.fhatos.org>"));
    FOS_TEST_OBJ_EQUAL(vri(""), OBJ_PARSER("<>"));
    FOS_TEST_OBJ_EQUAL(vri(":"), OBJ_PARSER("<:>"));
    FOS_TEST_OBJ_EQUAL(vri("a"), OBJ_PARSER("a"));
    FOS_TEST_OBJ_EQUAL(vri("../../a"), OBJ_PARSER("<../../a>"));
    FOS_TEST_OBJ_EQUAL(vri("abc/cba"), OBJ_PARSER("abc/cba"));
    FOS_TEST_OBJ_EQUAL(vri("aBc_cBa"), OBJ_PARSER("aBc_cBa"));
    //FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), OBJ_PARSER("uri[aaa_bbb/ccc/../ddd]"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), OBJ_PARSER("uri[<aaa_bbb/ccc/../ddd>]"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), OBJ_PARSER("<aaa_bbb/ccc/../ddd>"));
    //FOS_TEST_OBJ_NTEQL(vri("aaa_bbb/ccc/../ddd"), OBJ_PARSER("aaa_bbb/ccc/../ddd"));
  }

  void test_lst_parsing() {
    FOS_TEST_OBJ_EQUAL(lst(), OBJ_PARSER("[]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), OBJ_PARSER("[1,2,3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), OBJ_PARSER("[1   , 2 , 3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),lst({jnt(2),jnt(4)}),jnt(3)}), OBJ_PARSER("[1,[2,4],3]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       OBJ_PARSER("['a',['b',['e','c']],'d']"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       OBJ_PARSER("lst[['a',lst[['b',lst[['e','c']]]],'d']]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       OBJ_PARSER("lst[['a',['b',lst[['e','c']]],'d']]"));
  }

  void test_rec_parsing() {
    FOS_TEST_OBJ_EQUAL(rec(), OBJ_PARSER("[=>]"));
    TEST_ASSERT_EQUAL_STRING("[=>]", rec()->toString(NO_ANSI_PRINTER).c_str());
    FOS_TEST_OBJ_NTEQL(rec(), OBJ_PARSER("[]"));
    FOS_TEST_OBJ_EQUAL(rec(), OBJ_PARSER("rec[[=>]]"));
    FOS_TEST_OBJ_NTEQL(rec(), OBJ_PARSER("[a=>1]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),str("b")},{jnt(3),str("c")}}),
                       OBJ_PARSER("[1=>'a',2=>'b',3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),str("b")},{jnt(3),str("c")}}),
                       OBJ_PARSER("[  1  => 'a', 2=>  'b',3  =>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec()},{jnt(3),str("c")}}),
                       OBJ_PARSER("[1=>'a',2=>[=>],3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       OBJ_PARSER("[1=>'a',2=>['b'=>['d'=>4]],3=>'c']"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       OBJ_PARSER("rec[[1=>'a',2=>rec[['b'=>rec[['d'=>4]]]],3=>'c']]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),str("a")},{jnt(2),rec({{str("b"),rec({{str("d"),jnt(4)}})}})},{jnt(3),str("c")}}),
                       OBJ_PARSER("rec[[1=>'a',2=>['b'=>rec[['d'=>4]]],3=>'c']]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       OBJ_PARSER("rec[[1=>a,2=>[b=>[d=>4]],3=>c]]"));
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       OBJ_PARSER("rec\t  [\t[1 =>  a,2  =>[\t\t\tb =>[d => 4] ], 3=>   c]  ]"));
    FOS_TEST_OBJ_NTEQL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       OBJ_PARSER("rec[[2=>[b=>[d=>4]],3=>c]]")); // no 1=>a
  }

  void test_inst_parsing() {
    // TODO: TEST_ASSERT_EQUAL_STRING("play?play<=int[plus(2)]",
    //                  PROCESS("|play?play<=int[plus(2)]")->toString(SERIALIZER_PRINTER).
    //                c_str());

   // "<nat?nat<=int>|(x=>_,y=>2)[is(gt(0))]"
  }

  void test_inst_sugar_parsing() {
    ////////// x
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3 x 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(6), OBJ_PARSER("3x2")->apply());
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3x 2"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("  3x 2  "));
    //FOS_TEST_OBJ_NTEQL(jnt(6), OBJ_PARSER("3 x2")->apply());
    ////////// +
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3 + 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(5), OBJ_PARSER("3+2")->apply());
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3+ 2"));
    //FOS_TEST_OBJ_NTEQL(jnt(5), OBJ_PARSER("3 +2")->apply());
    // TODO FOS_TEST_OBJ_NTEQL(jnt(5), OBJ_PARSER("  map(3) + 2 x 2 .plus(-5) ")->apply());
    ////////// proto.map
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("9.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("{9}.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("start({9}).plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("map(9).plus(1)"));
    ////////// -< >-
    FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[+ b/]>-x c"))
    // TODO:     FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[_ + b/]>-x c"))
    FOS_TEST_OBJ_EQUAL(str("abc"), PROCESS("'a' + 'b' + 'c'"))
    ////////// _/x\_
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("[1]_/ x 3\\__/+ 5\\_>-"))
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("1-<[+ 2]_/ + 5\\_>-"))
  }

  void test_apply_mono_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.6"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.map(6)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(3)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(_)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(mult(1))"));
    // TODO: FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.  plus( mult ( 1)   )"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3     .plus(  mult( 1  )   )"));
  }

  void test_apply_poly_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[a=>6,b=>7].a"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[a=>7,b=>[c=>6]].b/c"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[7,6,5].<1>"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[8,7,6].<2>"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[8,[a=>[b=>6],c=>7]].<1>.a.b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[8,[a=>[b=>6],c=>7]].<1>.a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[  8  ,[   a=>[ b => 6],c   =>   7]   ].<1>.a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("/abc -> [8,[a=>[b=>6],c=>7]];.*</abc/1/a/b>"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("/abc -> [8,[a=>[b=>6],c=>7]]; */abc/1/a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("/abc -> [8,[a=>[b=>6],c=>7]]; */abc/1/a.b.to(/abc/x); */abc/x"));
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_noobj_parsing); //
    FOS_RUN_TEST(test_type_parsing); //
    FOS_RUN_TEST(test_bool_parsing); //
    FOS_RUN_TEST(test_int_parsing); //
    FOS_RUN_TEST(test_real_parsing); //
    FOS_RUN_TEST(test_str_parsing); //
    FOS_RUN_TEST(test_uri_parsing); //
    FOS_RUN_TEST(test_lst_parsing); //
    FOS_RUN_TEST(test_rec_parsing); //
    FOS_RUN_TEST(test_inst_parsing); //
    //////////////////////////////////
    FOS_RUN_TEST(test_inst_sugar_parsing); //
    FOS_RUN_TEST(test_apply_mono_parsing); //
    FOS_RUN_TEST(test_apply_poly_parsing); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
