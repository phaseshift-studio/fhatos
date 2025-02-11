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

  void test_tracker() {
    Tracker tracker = Tracker();
    TEST_ASSERT_TRUE(tracker.track("a.plus(b)")->closed());
    tracker.clear();
    TEST_ASSERT_FALSE(tracker.track("a.plus(b")->closed());
    tracker.clear();
    TEST_ASSERT_TRUE(tracker.track("func?int<=int(a=>45)[[a=>2].plus([b=>*a])]")->closed());
    tracker.clear();
    TEST_ASSERT_FALSE(tracker.track("func?int<=int(a=>45)[[a>2].plus([b=>*a])]")->closed());
    tracker.clear();
    TEST_ASSERT_TRUE(tracker.track("func?int{1,1}<=int{?}(a=>45)[[<a>=>2].plus([b=>*(<a>.plus(<../a>))])]")->closed());
  }

  void test_comment_parsing() {
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("--- a single line comment\n"));
    //FOS_TEST_OBJ_EQUAL(noobj(),  PROCESS("--- a single line comment"));
    FOS_TEST_ERROR("---\nsdfOo_asdf");
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("### sdfdasdf\n\t\nsdfsfasfsf ###"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("--- start \nint[1]"));
   // FOS_TEST_OBJ_EQUAL(str("bcd"),PROCESS("\n\n--- \n\n--- start \n\n\n### a comment ---\n--- end\n\n ###\nstr['bcd']"));
   // FOS_TEST_OBJ_EQUAL(str("abcd"),PROCESS("\n\n--- \n\n--- start \n\n\n'a'.plus(### a comment ---\n--- end\n\n ###\nstr['bcd'])"));
  }

  void test_type_parsing() {
    TEST_ASSERT_EQUAL(OType::NOOBJ, PROCESS("noobj")->otype);
  }

  void test_noobj_parsing() {
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("noobj"));
    FOS_TEST_OBJ_EQUAL(noobj(), PROCESS("noobj[noobj]"));
    // TODO: FOS_TEST_OBJ_EQUAL(vri(NOOBJ_FURI), PROCESS("noobj.type()"));
  }

  void test_bool_parsing() {
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("<bool>[false]"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("<bool>[   false]"));
    FOS_TEST_OBJ_NOT_EQUAL(dool(false), PROCESS("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("bool[start(false)]"));
    FOS_TEST_ERROR("< bool >[false]");
  }

  void test_int_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("1"));
    // TODO: weird bug FOS_TEST_OBJ_EQUAL(Obj::create(Any(),OType::OBJ,INT_FURI), PROCESS("int[]"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), PROCESS("-10"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), PROCESS("   -10   "));
    FOS_TEST_OBJ_EQUAL(jnt(-10,INT_FURI,id_p("/abc/ten")), PROCESS("   -10@/abc/ten   "));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("int[0]"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("int[1]"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("int   [ 0  ]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50), PROCESS("int?int<=int[-50]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50), PROCESS("int?int<=int    [     -50 ]"));
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
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), PROCESS("uri[<aaa_bbb/ccc/../ddd>]"));
    FOS_TEST_OBJ_EQUAL(vri("aaa_bbb/ccc/../ddd"), PROCESS("<aaa_bbb/ccc/../ddd>"));
  }

  void test_lst_parsing() {
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("[]"));
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("  [\n   ] "));
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("lst[[]]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), PROCESS("[1,2,3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2),jnt(3)}), PROCESS("[1   , 2 , 3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),lst({jnt(2),jnt(4)}),jnt(3)}), PROCESS("[1,[2,4],3]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("['a',['b',['e','c']],'d']"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("['a',  ['b'  ,[  'e'  ,'c']\n\n\n],  'd'\n\t]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("lst[['a',lst[['b',lst[['e','c']]]],'d']]"));
    FOS_TEST_OBJ_EQUAL(lst({str("a"),lst({str("b"),lst({str("e"),str("c")})}),str("d")}),
                       PROCESS("lst[['a',['b',lst[['e','c']]],'d']]"));
  }

  void test_rec_parsing() {
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS("[=>]"));
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS(" [   =>\n   ] "));
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS("rec[[=>]]"));
    TEST_ASSERT_EQUAL_STRING("[=>]", rec()->toString(NO_ANSI_PRINTER).c_str());
    FOS_TEST_OBJ_NTEQL(rec(), PROCESS("[]"));
    FOS_TEST_OBJ_EQUAL(rec(), PROCESS("rec[[=>]]"));
    FOS_TEST_OBJ_EQUAL(Obj::to_rec({{"a",jnt(1)},{"b",jnt(2)}}), PROCESS("[<a>=>1,<b>=>2]"));
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
    FOS_TEST_OBJ_EQUAL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       PROCESS("rec\t  [\t[1 =>  a,2  =>[\t\t\tb =>[d => 4] ], 3=>   c]  ]"));
    FOS_TEST_OBJ_NTEQL(rec({{jnt(1),vri("a")},{jnt(2),rec({{vri("b"),rec({{vri("d"),jnt(4)}})}})},{jnt(3),vri("c")}}),
                       PROCESS("rec[[2=>[b=>[d=>4]],3=>c]]")); // no 1=>a
  }

  void test_objs_parsing() {
    FOS_TEST_OBJ_EQUAL(Obj::to_objs(), PROCESS_ALL("{}"));
    FOS_TEST_OBJ_EQUAL(Obj::to_objs(), PROCESS_ALL("{   \n   } "));
  }

  void test_inst_parsing() {
    PROCESS("/abc/temp_inst -> |/abc/temp_inst?int<=int(a=>65)[plus(*a)]");
    // FOS_TEST_OBJ_EQUAL(jnt(66), PROCESS("1./abc/temp_inst()"));
    PROCESS("/abc/temp_inst -> |/abc/temp_inst?int<=int(a=>65)[plus(*a)]");
    // FOS_TEST_OBJ_EQUAL(jnt(68), PROCESS("2./abc/temp_inst(a=>66)"));
    PROCESS("/abc/temp_inst -> |/abc/temp_inst?int<=int(a=>65)[plus(*a)]");
    // FOS_TEST_OBJ_EQUAL(jnt(70), PROCESS("3./abc/temp_inst(67)"));
    FOS_TEST_FURI_EQUAL(ID("/abc/zyz"),
                        *PROCESS("/abc/temp_inst -> |/abc/temp_inst?int<=int(a=>65)[plus(*a)]@/abc/zyz")->vid);
    FOS_TEST_OBJ_EQUAL(jnt(73), PROCESS("4./abc/temp_inst(69)"));
    FOS_TEST_OBJ_EQUAL(jnt(73), PROCESS("4./abc/zyz(69)"));
    FOS_TEST_FURI_EQUAL(ID("/abc/zzz"),
                        *PROCESS("|/abc/temp_inst?int<=int(a=>65)[plus(*a)]@/abc/zzz")->vid);
    FOS_TEST_OBJ_EQUAL(jnt(105), PROCESS("5./abc/zzz(100)"));
    ///////////////////////////////////////////////////////////////
    FOS_TEST_OBJ_EQUAL(dool(false),
                       PROCESS(
                         "|(bool?bool<=bool(a=>_)[is(eq(*a))]@/abc/bool/true_static);\n"
                         "false./abc/bool/true_static(a=>_).to(/abc/bool/inst_parse);\n"
                         "*/abc/bool/inst_parse"
                       ));
    ///////////////////////////////////////////////////////////////
    const ID_p nat = id_p("/abc/nat");
    FOS_TEST_OBJ_EQUAL(jnt(5,nat), PROCESS("|/abc/nat?/abc/nat<=int[is(gt(0))]@/abc/nat; /abc/nat[5]"));
    FOS_TEST_OBJ_EQUAL(jnt(6,nat), PROCESS("|/abc/nat?/abc/nat<=int[is(gt(0))]@/abc/nat; /abc/nat[6]"));
    FOS_TEST_OBJ_EQUAL(jnt(7,nat), PROCESS("/abc/nat[7]"));
    FOS_TEST_OBJ_EQUAL(jnt(1000,nat), PROCESS("/abc/nat[1000]"));
    FOS_TEST_ERROR("/abc/nat[-12]");
    FOS_TEST_ERROR("/abc/nat[0]");

    const ID_p ncount = id_p("/abc/ncount");
    const Inst_p ncount_inst = PROCESS("|/abc/ncount?int{1}<=objs{*}(a=>7)[count().plus(*a)]@/abc/ncount");
    FOS_TEST_FURI_EQUAL(*ncount, ncount_inst->tid->query(""));
    FOS_TEST_FURI_EQUAL(*ncount, *ncount_inst->vid);
    TEST_ASSERT_EQUAL(OType::INST, ncount_inst->otype);
    TEST_ASSERT_TRUE(ncount_inst->tid->has_query(FOS_DOMAIN));
    TEST_ASSERT_TRUE(ncount_inst->tid->has_query(FOS_RANGE));
    TEST_ASSERT_TRUE(ncount_inst->tid->has_query(FOS_DOM_COEF));
    TEST_ASSERT_TRUE(ncount_inst->tid->has_query(FOS_RNG_COEF));
    FOS_TEST_FURI_EQUAL(*OBJS_FURI, *ncount_inst->domain());
    FOS_TEST_FURI_EQUAL(*INT_FURI, *ncount_inst->range());
    //  TE  const auto &[rmin,rmax] = this->range_coefficient();ST_ASSERT_EQUAL_STRING(ITypeSignatures.to_chars(IType::MANY_TO_ONE).c_str(),
    //                           ITypeSignatures.to_chars(ncount_inst->itype()).c_str());
    /* FOS_TEST_OBJ_EQUAL(jnt(10),PROCESS("{1,2,3}./abc/ncount()")); // default
     FOS_TEST_OBJ_EQUAL(jnt(10),PROCESS("{1,2,3}./abc/ncount(7)")); // position slotted
     FOS_TEST_OBJ_EQUAL(jnt(10),PROCESS("{1,2,3}./abc/ncount(a=>7)")); // key slotted
     FOS_TEST_OBJ_EQUAL(jnt(11),PROCESS("{1,2,3}./abc/ncount(8)"));
     FOS_TEST_OBJ_EQUAL(jnt(12),PROCESS("{1,2,3}./abc/ncount(a=>9)"));
     FOS_TEST_ERROR("{1,2,3}./abc/ncount(a)");*/

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
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3x 2"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3x2"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3 x2"));
    ////////// +
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3 + 2"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3+ 2"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3+2"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("3 +2"));
    //FOS_TEST_OBJ_NTEQL(jnt(5), PROCESS("  start(3) + 2 x 2 .plus(-5) "));
    ////////// proto.map
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("9.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("{9}.plus(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("start({9}).plus(1)"));
    FOS_TEST_ERROR("map(9).plus(1)");
    FOS_TEST_OBJ_EQUAL(str("abc"), PROCESS("'a' + 'b' + 'c'"))
    ////////// -< >-
    FOS_TEST_OBJ_EQUAL(lst({vri("a"),jnt(1),vri("a/b")}), PROCESS("a-<[_,1,+ /b]"))
    FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[+ b/]>-x c"))
    FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("a-<[_ + b/]>-x c"))
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("a-<[_,-<[_,-<[_,_,_]]].count()"));
    FOS_TEST_OBJ_EQUAL(jnt(2), PROCESS("a-<[_,-<[_,-<[_,_,_]]]>-.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("a-<[_,-<[_,-<[_,_,_]]]>-.>-.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("a-<[_,-<[_,-<[_,_,_]]]>-.>-.>-.count()"));
    ////////// _/x\_
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("[1]_/ x 3\\__/+ 5\\_>-"))
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("1-<[+ 2]_/ + 5\\_>-"))
    FOS_TEST_OBJ_EQUAL(str("a b c "), PROCESS("'abc'_/plus(' ')\\_"))
    ////////// _]x[_
    FOS_TEST_OBJ_EQUAL(Obj::to_objs({jnt(2),jnt(3),jnt(4)}), PROCESS_ALL("{1,2,3}_]plus(1)[_"))
    FOS_TEST_OBJ_EQUAL(Obj::to_objs({jnt(6)}), PROCESS_ALL("{1,2,3}_]{count()}[_.plus(3)"))
    ////////// |
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("start(1).8"))
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("1|8"))
    // TODO: FOS_TEST_OBJ_EQUAL(BCODE_FURI, PROCESS("1|plus(7)")->tid);
    ////////// ==
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4),jnt(6)}), PROCESS("[1,2,3]==[x 2,mult(2),x 2]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4),jnt(6)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(2),jnt(4)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[_,_]"));
    FOS_TEST_OBJ_NTEQL(lst({jnt(2),jnt(4)}), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[+ 0,plus(1)]"));
    FOS_TEST_OBJ_EQUAL(lst(), PROCESS("[1,2,3]==[+ 1,+ 2,+ 3]==[]"));
    ////////// @
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("10@/abc/at.mult(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("10@</abc/at>.mult(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("10.@</abc/at>.mult(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("10.mult(10).at(/abc/at)"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("10.mult(10).to(/abc/at).at(/abc/at)"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS("/abc/at->(map(10).mult(10)).@/abc/at"));
    FOS_TEST_OBJ_EQUAL(jnt(100,INT_FURI,id_p("/abc/at")), PROCESS(" /abc/at -> (\n\tmap(10).mult(\n10)  ).@\t/abc/at"));
    ////////// *
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("</abc/o1o>->101"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("/abc/o1o -> 101"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("/abc/o1o->101"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("*/abc/o1o"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("*(/abc/o1o)"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("*    (   /abc/o1o )"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("*(/abc/o12,101)"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("*(/abc/o12,  101)"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("</abc/o1o>.*()"));
    FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("</abc/o1o>.*(_)"));
    // TODO: maybe not though: FOS_TEST_OBJ_EQUAL(jnt(101), PROCESS("</abc/o1o>*"));
    FOS_TEST_OBJ_EQUAL(jnt(123456), PROCESS(
                         "</abc/star_1> -> /abc/star_2;"
                         "/abc/star_2 -> </abc/star_3>;"
                         "/abc/star_3 -> /abc/star_4;"
                         "/abc/star_4 -> 123456;"
                         "****/abc/star_1"));
  }

  void test_apply_mono_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.6"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'   .  6"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("'abc'.map(6)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(3)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(_)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.plus(mult(1))"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3.  plus( mult ( 1)   )"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("3     .plus(  mult( 1  )   )"));
  }

  void test_apply_poly_parsing() {
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("rec[[a=>1,b=>7]].a"));
    FOS_TEST_OBJ_EQUAL(jnt(2), PROCESS("[a=>7,b=>rec[[c=>2]]].b/c"));
    FOS_TEST_OBJ_EQUAL(jnt(3), PROCESS("[7,3,5].<1>"));
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("[8,7,4].<2>"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("lst[[8,[a=>rec[[b=>5]],c=>7]]].<1>.a.b"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("[8,rec[[a=>[b=>6],c=>7]]].<1>.a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(7), PROCESS("[  8  ,[   a=>[ b => 7],c   =>   7]   ].<1>.a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(8), PROCESS("/abc -> lst[[8,[a=>[b=>8],c=>7]]];.*</abc/1/a/b>"));
    FOS_TEST_OBJ_EQUAL(jnt(9), PROCESS("/abc -> [8,[a=>[b=>9],c=>7]]; */abc/1/a/b"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("/abc -> lst[[8,[a=>[b=>6],c=>7]]]; */abc/1/a.b.to(/abc/2); */abc/2 + -6"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("/abc -> [8,[a=>[b=>6],c=>7]];*/abc/1/a.b.to(/abc/2);*/abc/2 + -6"));
    FOS_TEST_OBJ_EQUAL(
        jnt(0), PROCESS("/abc -> [8,rec[[a=>rec[[b=>6]],c=>7]]];\n*/abc/1/a.b.to(/abc/2);\n*/abc/2 + -6\n"));
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_comment_parsing); //
      FOS_RUN_TEST(test_tracker); //

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
      FOS_RUN_TEST(test_objs_parsing); //
      //////////////////////////////////
      FOS_RUN_TEST(test_inst_sugar_parsing); //
      FOS_RUN_TEST(test_apply_mono_parsing); //
      FOS_RUN_TEST(test_apply_poly_parsing); //
      )
} // namespace fhatos

SETUP_AND_LOOP();
