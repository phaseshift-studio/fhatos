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
    args->rec_value()->insert({vri("_0"),jnt(10)});
     TEST_ASSERT_TRUE(args->is_indexed_args());
    args->rec_value()->insert({vri("_1"),str("eleven")});
     TEST_ASSERT_TRUE(args->is_indexed_args());
     args->rec_value()->insert({vri("_3"),vri("BAD")});
    TEST_ASSERT_FALSE(args->is_indexed_args());
    //TODO: FOS_TEST_OBJ_EQUAL(args,PROCESS("[_0=>10,_1=>'eleven',_3=>BAD]"));
    }

  void test_plus_inst() {
  	// bool
    FOS_TEST_OBJ_EQUAL(dool(true),PROCESS("true.plus(false)"));
   	FOS_TEST_OBJ_EQUAL(dool(true),PROCESS("bool[false].plus(true)"));
    FOS_TEST_OBJ_EQUAL(dool(true),PROCESS("false.plus(bool[true])"));
    FOS_TEST_OBJ_EQUAL(dool(true),PROCESS("true.plus(plus(plus(plus(_))))"));
    FOS_TEST_OBJ_EQUAL(dool(false),PROCESS("false.plus(plus(plus(plus(_))))"));
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
    FOS_TEST_OBJ_EQUAL(objs({vri("a/x"),vri("/b/x"),vri("/b/x"),vri("cd_e/x")}), PROCESS_ALL("{a,/b/,/b/,cd_e}.plus(x)"));
    FOS_TEST_OBJ_EQUAL(str("1245"), PROCESS("str['1'].plus('245')"));
    FOS_TEST_OBJ_EQUAL(vri("a1/b245"), PROCESS("a1.plus(<b245>)"));
    FOS_TEST_OBJ_EQUAL(vri("a/a/a/a/a/a"), PROCESS("<a>.plus(plus(plus(plus(plus(_)))))"));
  }

  void test_mult_inst() {
    // uri
	FOS_TEST_OBJ_EQUAL(vri("/b"), PROCESS("<a>.mult(mult(mult(mult(mult(<../b>)))))"));
  }

  void test_count_inst() {
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("{1,2,2,67}.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("35.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(0), PROCESS("35.is(gt(40)).count()"));
    FOS_TEST_OBJ_EQUAL(jnt(2), PROCESS("{67,35,2465}.is(gt(40)).count()"));
   FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("1-<[_,_,_,_,_]>-.count()"));
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_inst_args); //
    FOS_RUN_TEST(test_plus_inst); //
    FOS_RUN_TEST(test_count_inst); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
