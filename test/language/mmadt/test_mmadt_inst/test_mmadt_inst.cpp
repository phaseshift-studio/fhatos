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
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_MMADT_EXT_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /abc/#
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_as_inst() {
    FOS_TEST_FURI_EQUAL(ID("/abc/nat?dom=/mmadt/int&dc=1,1&rng=/mmadt/int&rc=0,1"),
                        *PROCESS("/abc/nat -> |/abc/nat?int{?}<=int{1,1}()[is(gt(0))]")->tid);
    FOS_TEST_OBJ_EQUAL(Obj::to_int(5), PROCESS("5.as(int)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(5), PROCESS("5.as(/mmadt/int)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(5), PROCESS("5.as(type(_))"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("5.as(noobj)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), PROCESS("5.as(/mmadt/noobj)"));
    //FOS_TEST_OBJ_EQUAL(Obj::to_int(5), PROCESS("5.as([int][])"));
    //FOS_TEST_OBJ_EQUAL(Obj::to_int(5), PROCESS("5.as([int][is(gt(0))])"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(5,id_p("/abc/nat")), PROCESS("/abc/nat[5]"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(1,id_p("/abc/nat")), PROCESS("/abc/nat[5].minus(4)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(20,id_p("/abc/nat")), PROCESS("/abc/nat[5].mult(4)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(20), PROCESS("5.mult(/abc/nat[4])"));
    // FOS_TEST_OBJ_EQUAL(Obj::to_int(5,id_p("/abc/nat")), PROCESS("5.[/abc/nat][]"));
    // FOS_TEST_OBJ_EQUAL(Obj::to_int(5,id_p("/abc/nat")), PROCESS("5.map([/abc/nat][])"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(55,id_p("/abc/nat")), PROCESS("55.as(/abc/nat)"));
    //FOS_TEST_OBJ_EQUAL(Obj::to_int(555,id_p("/abc/nat")), PROCESS("555.as([/abc/nat][])"));
    //FOS_TEST_OBJ_EQUAL(Obj::to_int(5555,id_p("/abc/nat")), PROCESS("5555.map(as([/abc/nat][]))"));
    //FOS_TEST_OBJ_EQUAL(Obj::to_int(55555,id_p("/abc/nat")), PROCESS("55555.as(map(as([/abc/nat][])))"));
    FOS_TEST_ERROR("-5.as(/abc/nat)");
    FOS_TEST_ERROR("5.as(/abc/nat).minus(6)");
    FOS_TEST_ERROR("5.as(/abc/nat).minus(/abc/nat[6])");
    FOS_TEST_ERROR("5.as(/abc/nat).mult(-6)");
    //  FOS_TEST_ERROR("-55.map([/abc/nat][])");
    //  FOS_TEST_ERROR("-555.as([/abc/nat][])");
    //  FOS_TEST_ERROR("-5555.map(as([/abc/nat][]))");
    FOS_TEST_ERROR("-5555.as(str)");
    // TODO: FOS_TEST_ERROR("-5.as([int][is(gt(0))])");
    //////////////////////////////
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(true), PROCESS("'true'.as(bool)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(false), PROCESS("0.as(bool)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(true), PROCESS("1.as(bool)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_bool(true), PROCESS("true.as(bool)"));
    /////
    FOS_TEST_OBJ_EQUAL(Obj::to_int(46), PROCESS("'46'.as(int)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(46), PROCESS("real[46.1].as(int)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_int(46), PROCESS("46.as(int)"));
    /////
    TEST_ASSERT_TRUE(PROCESS("'47.3'.as(real)")->real_value() > 47.2 &&
        PROCESS("'47.3'.as(real)")->real_value() < 47.4);
    FOS_TEST_OBJ_EQUAL(Obj::to_real(47.3), PROCESS("real[47.3].as(real)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_real(47.0), PROCESS("47.as(real)"));
    /////
    FOS_TEST_OBJ_EQUAL(Obj::to_str("true"), PROCESS("true.as(str)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_str("13"), PROCESS("13.as(str)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_str("-13"), PROCESS("-13.as(str)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_str("hello"), PROCESS("'hello'.as(str)"));
    FOS_TEST_OBJ_EQUAL(Obj::to_str("46.23000"), PROCESS("real[46.23].as(str)"));
  }

  void test_ref_inst() {
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("/abc/b->10"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("*/abc/b"));
    FOS_TEST_OBJ_EQUAL(jnt(20), PROCESS("/abc/b-->20"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("*/abc/b"));
  }

  void test_inst_args() {
    const InstArgs args = Obj::to_inst_args();
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("0"), jnt(10)});
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("1"), str("eleven")});
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("3"), vri("BAD")});
    TEST_ASSERT_FALSE(args->is_indexed_args());
    FOS_TEST_OBJ_EQUAL(args, PROCESS("[<0>=>10,<1>=>'eleven',<3>=>BAD]"));
    args->rec_value()->insert({vri("2"), vri("GOOD")});
    TEST_ASSERT_FALSE(args->is_indexed_args());
    FOS_TEST_OBJ_EQUAL(args, PROCESS("[<0>=>10,<1>=>'eleven',<3>=>BAD,<2>=>GOOD]"));
    //////////////////////
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5./abc/temp?int<=int(a=>10)[plus(*a)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5.<?int<=int>(a=>10)[plus(*a)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15),PROCESS("5.?int<=int()[plus(10)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15),PROCESS("5.()[plus(10)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5.<?>[plus(10)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5.< >[plus(10)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5.<>(a=>10)[plus(*a)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5./abc/temp(a=>10)[plus(*a)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15), PROCESS("5./abc/temp()[plus(10)]"));
    FOS_TEST_OBJ_EQUAL(jnt(15),PROCESS("5./abc/temp?int<=int(10)[plus(from(<0>))]"));
  }

  void test_barrier_inst() {
    // objs
    FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(3),jnt(4),jnt(5)}), PROCESS_ALL("{1,2,3}.barrier(plus(2))"));
    //FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(4)}), PROCESS_ALL("{7,109,18,566}.barrier(count())"));
    // FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(7+109+18+566)}), PROCESS_ALL("{7,109,18,566}.barrier(sum())"));
  }

  void test_sum_inst() {
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("{true,false,false}.sum()"));
    FOS_TEST_OBJ_EQUAL(jnt(10), PROCESS("{2,5,3}.sum()"));
    FOS_TEST_OBJ_EQUAL(str("fhat"), PROCESS("{'f','ha','t'}.sum()"));
    FOS_TEST_OBJ_EQUAL(lst({vri("a/b/c")}), PROCESS("[a,b,c]._/sum()\\_"));
    FOS_TEST_OBJ_EQUAL(vri("a/b/c"), PROCESS("{a,b,c}.sum()"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),str("a"),dool(false),str("b")}), PROCESS("{[1],['a',false],['b']}.sum()"));
  }

  void test_prod_inst() {
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("{true,false,false}.prod()"))
    FOS_TEST_OBJ_EQUAL(jnt(30), PROCESS("{2,5,3}.prod()"));
    FOS_TEST_OBJ_EQUAL(jnt(30), PROCESS("{2,5,3}.prod()"));
  }

  void test_choose_inst() {
    FOS_TEST_OBJ_EQUAL(lst({jnt(1),Obj::to_noobj(),Obj::to_noobj()}), PROCESS("1-|[_,_,_]"));
    FOS_TEST_OBJ_EQUAL(lst({Obj::to_noobj(),jnt(1),Obj::to_noobj()}), PROCESS("1-|[is(gt(2)),_,_]"));
    FOS_TEST_OBJ_EQUAL(lst({Obj::to_noobj(),jnt(11,id_p("/mmadt/ext/nat")),Obj::to_noobj()}),
                       PROCESS("1-|[is(gt(2)),plus(10).as(nat),plus(1)]"));
    ///////////
    FOS_TEST_OBJ_EQUAL(PROCESS("[1=>1,is(gt(0))=>noobj,_=>noobj]"), PROCESS("1-|[1=>_,is(gt(0))=>_,_=>_]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[is(gt(2))=>noobj,is(gt(0))=>1,_=>noobj]"), PROCESS("1-|[is(gt(2))=>_,is(gt(0))=>_,_=>_]"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[is(gt(2))=>noobj,is(gt(0))=>11,_=>noobj]"), PROCESS("1-|[is(gt(2))=>_,is(gt(0))=>plus(10),_=>_]"));
  }

  void test_merge_inst() {
    FOS_TEST_OBJ_EQUAL(objs({jnt(1),jnt(2),jnt(3)}), PROCESS_ALL("[[[1,2,3]]]>->->-"))
    FOS_TEST_OBJ_EQUAL(objs({vri("a/d"),vri("b/d"),vri("c/d")}), PROCESS_ALL("[a,b,c]>-+d"));
    // FOS_TEST_OBJ_EQUAL(objs({jnt(9),vri("b/c"),dool(true)}),PROCESS_ALL("[4=>+5,b=>+c,'fhat'=>true]>-"));
  }

  /*void test_reduce_inst() {
  	FOS_TEST_OBJ_EQUAL(dool(true),PROCESS("{true,false,false}.reduce(|plus())"));
    FOS_TEST_OBJ_EQUAL(dool(false),PROCESS("false.reduce(plus())"));
  	FOS_TEST_OBJ_EQUAL(jnt(10),PROCESS("{2,5,3}.reduce(*plus)"));
    FOS_TEST_OBJ_EQUAL(jnt(2),PROCESS("2.reduce(*plus)"));
    FOS_TEST_OBJ_EQUAL(str("fhat"),PROCESS("{'f','ha','t'}.reduce(|plus())"));
    FOS_TEST_OBJ_EQUAL(str("f"),PROCESS("'f'.reduce(|plus())"));
  }*/

  void test_uri_lshift_inst() {
    FOS_TEST_OBJ_EQUAL(PROCESS("a/b/c"), PROCESS("a/b/c<<0"));
    FOS_TEST_OBJ_EQUAL(PROCESS("b/c"), PROCESS("a/b/c<<1"));
    FOS_TEST_OBJ_EQUAL(PROCESS("c"), PROCESS("a/b/c<<2"));
    FOS_TEST_OBJ_EQUAL(PROCESS("<>"), PROCESS("a/b/c<<3"));
    FOS_TEST_OBJ_EQUAL(PROCESS("<>"), PROCESS("a/b/c<<4"));
    ///
    FOS_TEST_OBJ_EQUAL(PROCESS("a/b/c"), PROCESS("a/b/c<<(<>)"));
    FOS_TEST_OBJ_EQUAL(PROCESS("b/c"), PROCESS("a/b/c<<a"));
    FOS_TEST_OBJ_EQUAL(PROCESS("c"), PROCESS("a/b/c<<a/b"));
    FOS_TEST_OBJ_EQUAL(PROCESS("<>"), PROCESS("a/b/c<<a/b/c"));
    // TODO:  FOS_TEST_OBJ_EQUAL(PROCESS("<>"), PROCESS("a/b/c<<a/b/c/d"));
  }

  void test_rec_lshift_inst() {
    FOS_TEST_OBJ_EQUAL(PROCESS("[a/b/c=>1,a/b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<0"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[b/c=>1,b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<1"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[b/c=>1,b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<2"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<.<<"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<3"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<.<<.<<"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<4"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<.<<.<<.<<"));
    //
    // FOS_TEST_OBJ_EQUAL(PROCESS("[a/b/c=>1,a/b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<(|<>)"));
    // FOS_TEST_OBJ_EQUAL(PROCESS("[a/b/c=>1,a/b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|(<>)"));
    // FOS_TEST_OBJ_EQUAL(PROCESS("[b/c=>1,b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<(|<.>)"));
    // FOS_TEST_OBJ_EQUAL(PROCESS("[b/c=>1,b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|<.>"));
    /*FOS_TEST_OBJ_EQUAL(PROCESS("[b/c=>1,b/d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|a"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<(|<a/b>)"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|<a/b>"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<(|<a/b>)"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|(<a/b>)"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a/b/c=>1,a/b/d=>2].<<|<a/b>"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|a/b/c"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a/b/c=>1,a/b/d=>2]<<|a/b/d"));*/
    ////
    /*FOS_TEST_OBJ_EQUAL(PROCESS("[a=>[b=>[c=>1,d=>2]]]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<0"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[b=>[c=>1,d=>2]]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<1"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<2"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<3"));
    //
    FOS_TEST_OBJ_EQUAL(PROCESS("[a=>[b=>[c=>1,d=>2]]]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<(<.>)"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[b=>[c=>1,d=>2]]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<a"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[c=>1,d=>2]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<a/b"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<a/b/c"));
    FOS_TEST_OBJ_EQUAL(PROCESS("[=>]"), PROCESS("[a=>[b=>[c=>1,d=>2]]]<<a/b/d"));*/
  }

  void test_rshift_inst() {

  }

  void test_neg_inst() {
    // bool
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("neg(false)"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("false.neg()"));
    FOS_TEST_OBJ_EQUAL(dool(true,id_p("truth")), PROCESS("truth -> [bool][]; neg(truth[false])"));
    FOS_TEST_OBJ_EQUAL(dool(true,id_p("truth")), PROCESS("truth -> [bool][]; truth[false].neg()"));
    // int
    FOS_TEST_OBJ_EQUAL(jnt(-1), PROCESS("neg(1)"));
    FOS_TEST_OBJ_EQUAL(jnt(-2), PROCESS("2.neg()"));
    FOS_TEST_OBJ_EQUAL(jnt(3,id_p("number")), PROCESS("number -> int[is(neq(0))]; neg(number[-3])"));
    FOS_TEST_OBJ_EQUAL(jnt(4,id_p("number")), PROCESS("number -> int[is(neq(0))]; number[-4].neg()"));
  }

  void test_plus_inst() {
    // bool
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true.plus(false)"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("bool[false].plus(true)"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("false.plus(bool[true])"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true.plus(plus(plus(plus(_))))"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false.plus(plus(plus(plus(_))))"));
    // int
    FOS_TEST_OBJ_EQUAL(objs({jnt(11),jnt(12),jnt(12),jnt(77)}), PROCESS_ALL("{1,2,2,67}.plus(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(246), PROCESS("1.plus(245)"));
    FOS_TEST_OBJ_EQUAL(jnt(6), PROCESS("1.plus(plus(plus(plus(plus(_)))))"));
    // str
    FOS_TEST_OBJ_EQUAL(objs({str("ax"),str("bx"),str("bx"),str("cdex")}), PROCESS_ALL("{'a','b','b','cde'}.plus('x')"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("str['1'].plus('245')"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("'1'.plus(str['245'])"));
    FOS_TEST_OBJ_EQUAL(str("aaaaaa"), PROCESS("'a'.plus(plus(plus(plus(plus(_)))))"));
    // uri
    FOS_TEST_OBJ_EQUAL(objs({vri("a/x"),vri("/b/x"),vri("/b/x"),vri("cd.e/x")}),
                       PROCESS_ALL("{a,/b/,/b/,<cd.e>}.plus(x)"));
    FOS_TEST_OBJ_EQUAL(objs({vri("a/z"),vri("/b/z"),vri("/b/z"),vri("cd_e/z")}),
                       PROCESS_ALL("{a,/b/,/b/,cd_e}.plus(z)"));
    FOS_TEST_OBJ_EQUAL(vri("a1/b245"), PROCESS("a1.plus(<b245>)"));
    FOS_TEST_OBJ_EQUAL(vri("a/a/a/a/a/a"), PROCESS("<a>.plus(plus(plus(plus(plus(_)))))"));
  }

  void test_minus_inst() {
    // bool
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true.minus(false)"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("bool[false].minus(true)"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false.minus(bool[true])"));
    //FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("true.minus(minus(minus(_)))"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("false.minus(minus(minus(_)))"));
    // int
    FOS_TEST_OBJ_EQUAL(objs({jnt(-9),jnt(-8),jnt(-8),jnt(57)}), PROCESS_ALL("{1,2,2,67}.minus(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(-244), PROCESS("1.minus(245)"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("1.minus(minus(minus(minus(minus(_)))))"));
    // str
    FOS_TEST_OBJ_EQUAL(objs({str("aa"),str(""),str("aa"),str("acde")}),
                       PROCESS_ALL("{'aab','bbb','aba','acde'}.minus('b')"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("str['1'].plus('245')"));
    FOS_TEST_OBJ_EQUAL(str("1"), PROCESS("'1'.minus(str['245'])"));
    FOS_TEST_OBJ_EQUAL(str(""), PROCESS("'a'.minus(minus(minus(minus(minus(_)))))"));
    // uri
    FOS_TEST_OBJ_EQUAL(objs({vri("a/x"), vri("/b/x"), vri("/b/x"), vri("cd.e/x")}), PROCESS_ALL(
                         "{a,/b/,/b/,<cd.e>}.plus(x)"));
    FOS_TEST_OBJ_EQUAL(objs({vri("a/x"),vri("/b/x"),vri("/b/x"),vri("cd_e/x")}),
                       PROCESS_ALL("{a,/b/,/b/,cd_e}.plus(x)"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("str['1'].plus('245')"));
    FOS_TEST_OBJ_EQUAL(vri("a1/b245"), PROCESS("a1.plus(<b245>)"));
    FOS_TEST_OBJ_EQUAL(vri("a/a/a/a/a/a"), PROCESS("<a>.plus(plus(plus(plus(plus(_)))))"));
  }

  void test_mult_inst() {
    // bool
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("true.mult(false)"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("bool[false].mult(true)"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false.mult(bool[true])"));
    FOS_TEST_OBJ_EQUAL(dool(true), PROCESS("true.mult(mult(mult(mult(_))))"));
    FOS_TEST_OBJ_EQUAL(dool(false), PROCESS("false.mult(mult(mult(mult(_))))"));
    // int
    FOS_TEST_OBJ_EQUAL(objs({jnt(10),jnt(20),jnt(20),jnt(670)}), PROCESS_ALL("{1,2,2,67}.mult(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(245), PROCESS("1.mult(245)"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("1.mult(mult(mult(mult(mult(_)))))"));
    FOS_TEST_OBJ_EQUAL(jnt(46656), PROCESS("6.mult(mult(mult(mult(mult(_)))))"));
    // uri
    FOS_TEST_OBJ_EQUAL(vri("/b"), PROCESS("<a>.mult(mult(mult(mult(mult(<../b>)))))"));
  }

  void test_count_inst() {
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("{1,2,2,67}.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("35.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("35.is(gt(40)).count()"));
    FOS_TEST_OBJ_EQUAL(jnt(2), PROCESS("{67,35,2465}.is(gt(40)).count()"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("1-<[_,_,_,_,_]>-.count()"));
    // FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("'fhat'-<[_,_,_,_,_]>-_]{count()}[_"));
  }

  void test_drop_inst() {
    FOS_TEST_FURI_EQUAL(fURI(MMADT_SCHEME "/from"), *PROCESS("/abc/drop_1 -> |*/abc/drop_2")->tid);
    FOS_TEST_FURI_EQUAL(fURI("plus"), *PROCESS("/abc/drop_2 -> |plus(10)")->tid);
    // FOS_TEST_OBJ_EQUAL(jnt(33), PROCESS("23.drop(drop(*/abc/drop_1))"));
    // TODO: implement repeat(drop()).until(not_code)   drop_hard() :)
  }

  void test_obj_inst() {
    PROCESS("/abc/k -> 42");
    FOS_TEST_OBJ_EQUAL(jnt(42), PROCESS("/abc/k()"));
    FOS_TEST_OBJ_EQUAL(jnt(42), PROCESS("/abc/k(1,2)"));
    FOS_TEST_OBJ_EQUAL(jnt(42), PROCESS("/abc/k(/abc/a=>1,/abc/b=>2)"));
    //////////////////////////////////////////////////////////////////
    //PROCESS("/abc/k -> |plus(10)");
    //FOS_TEST_OBJ_EQUAL(jnt(11), PROCESS("1./abc/k()"));
    //FOS_TEST_OBJ_EQUAL(jnt(12), PROCESS("2./abc/k(1,2)"));
    //////////////////////////////////////////////////////////////////
    PROCESS("/abc/k -> |(plus(10).plus(1))");
    FOS_TEST_OBJ_EQUAL(jnt(12), PROCESS("1./abc/k()"));
    FOS_TEST_OBJ_EQUAL(jnt(13), PROCESS("2./abc/k(1,2)"));
    //////////////////////////////////////////////////////////////////
    PROCESS("/abc/k -> |(plus(*<0>).plus(*<1>))");
    FOS_TEST_OBJ_EQUAL(jnt(7), PROCESS("1./abc/k(2,4)"));
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("1./abc/k(<0>=>1,<1>=>2)"));
    //////////////////////////////////////////////////////////////////
    PROCESS("/abc/k -> |[*a1,*b1,_]");
    FOS_TEST_OBJ_EQUAL(lst({jnt(32),str("pig"),jnt(12)}), PROCESS("12./abc/k(a1=>32,b1=>'pig')"));
    FOS_TEST_ERROR("1./abc/k(88,'fhatos')");
    PROCESS("/abc/k -> |[*<0>,*<1>,_]");
    FOS_TEST_OBJ_EQUAL(lst({jnt(88),str("fhatos"),jnt(11)}), PROCESS("11./abc/k(88,'fhatos')"));
    FOS_TEST_OBJ_EQUAL(lst({jnt(100),vri("test"),jnt(10)}), PROCESS("10./abc/k(/abc/a=>100,/abc/b=>test)"));
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_as_inst); //
      FOS_RUN_TEST(test_ref_inst); //
      FOS_RUN_TEST(test_inst_args); //
      FOS_RUN_TEST(test_uri_lshift_inst); //
      FOS_RUN_TEST(test_rec_lshift_inst); //
      FOS_RUN_TEST(test_rshift_inst); //
      FOS_RUN_TEST(test_barrier_inst); //
      FOS_RUN_TEST(test_sum_inst); //
      FOS_RUN_TEST(test_prod_inst); //
      FOS_RUN_TEST(test_choose_inst); //
      FOS_RUN_TEST(test_merge_inst); //
      //FOS_RUN_TEST(test_reduce_inst); //
      //FOS_RUN_TEST(test_neg_inst); //
      FOS_RUN_TEST(test_plus_inst); //
      FOS_RUN_TEST(test_minus_inst); //
      FOS_RUN_TEST(test_mult_inst); //
      FOS_RUN_TEST(test_count_inst); //
      //FOS_RUN_TEST(test_drop_inst); //
      FOS_RUN_TEST(test_obj_inst); //
      )
} // namespace fhatos

SETUP_AND_LOOP();
