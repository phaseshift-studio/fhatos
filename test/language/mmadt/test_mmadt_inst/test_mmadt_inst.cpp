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

#ifndef mmadt_test_mmadt_inst_cpp
#define mmadt_test_mmadt_inst_cpp
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

  void test_inst_args() {
    InstArgs args = Obj::to_inst_args();
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("_0"), jnt(10)});
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("_1"), str("eleven")});
    TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("_3"), vri("BAD")});
    TEST_ASSERT_FALSE(args->is_indexed_args());
    //FOS_TEST_OBJ_EQUAL(args,PROCESS("[_0=>10,_1=>'eleven',_3=>BAD]"));
  }

    void test_barrier_inst() {
      // objs
      FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(3),jnt(4),jnt(5)}), PROCESS_ALL("{1,2,3}.barrier(plus(2))"));
      FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(4)}), PROCESS_ALL("{7,109,18,566}.barrier({count()})"));
     // FOS_TEST_OBJ_EQUAL(Objs::to_objs({jnt(7+109+18+566)}), PROCESS_ALL("{7,109,18,566}.barrier({sum()})"));
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
    // TODO: FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("str['1'].plus('245')"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("'1'.plus(str['245'])"));
    FOS_TEST_OBJ_EQUAL(str("aaaaaa"), PROCESS("'a'.plus(plus(plus(plus(plus(_)))))"));
    // uri
    // TODO: FOS_TEST_OBJ_EQUAL(objs({vri("a/x"),vri("/b/x"),vri("/b/x"),vri("cd.e/x")}), PROCESS_ALL("{a,/b/,/b/,cd.e}.plus(x)"));
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
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("'fhat'-<[_,_,_,_,_]>-_]{count()}[_"));
  }

  void test_drop_inst() {
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(MMADT_SCHEME "/from"), *PROCESS("/abc/drop_1 -> |*/abc/drop_2")->tid());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(MMADT_SCHEME "/plus"), *PROCESS("/abc/drop_2 -> |plus(10)")->tid());
    FOS_TEST_OBJ_EQUAL(jnt(33), PROCESS("23.drop(drop(*/abc/drop_1))"));
    // TODO: implement repeat(drop()).until(not_code)   drop_hard() :)
  }


  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_inst_args); //
    FOS_RUN_TEST(test_barrier_inst); //
    //FOS_RUN_TEST(test_neg_inst); //
    FOS_RUN_TEST(test_plus_inst); //
    FOS_RUN_TEST(test_mult_inst); //
    FOS_RUN_TEST(test_count_inst); //
    FOS_RUN_TEST(test_drop_inst); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
