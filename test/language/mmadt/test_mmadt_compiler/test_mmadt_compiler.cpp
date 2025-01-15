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
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /compiler/#
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_type_check_base_types() {
    Compiler compiler = Compiler();
    compiler.throw_on_miss = false;
    FOS_TEST_COMPILER_TRUE(dool(true),BOOL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(jnt(1),INT_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(real(1.2),REAL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(str("one.two"),STR_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(vri("http://one/two"),URI_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(lst({jnt(1),str("point"),real(2.0)}),LST_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(rec({{jnt(1),str("point")},{real(2.0),dool(true)}}),REC_FURI,compiler.type_check);
    ////
    FOS_TEST_COMPILER_FALSE(dool(true),INT_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(jnt(1),REAL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(1.2),BOOL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(str("one.two"),URI_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(vri("http://one/two"),STR_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(lst({jnt(1),str("point"),real(2.0)}),REC_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(rec({{jnt(1),str("point")},{real(2.0),dool(true)}}),LST_FURI,compiler.type_check);
  }

  void test_type_check_derived_mono_types() {
    Compiler compiler = Compiler();
    compiler.throw_on_miss = false;
    // bool
    TYPE_SAVER(id_p("/compiler/truth"),dool(true));
    FOS_TEST_COMPILER_TRUE(dool(true),id_p("/compiler/truth"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(dool(false),id_p("/compiler/truth"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(jnt(43),id_p("/compiler/truth"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(str("true"),id_p("/compiler/truth"),compiler.type_check);
    // int
    TYPE_SAVER(id_p("/compiler/nat"),mmadt::Parser::singleton()->parse("is(gt(0))"));
    FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(jnt(-1),id_p("/compiler/nat"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(1.46),id_p("/compiler/nat"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(-1.46),id_p("/compiler/nat"),compiler.type_check);
   PROCESS("/compiler/nat2 -> |/compiler/nat2?int<=int(=>)[is(gt(0))]");
    FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat2"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(jnt(-1),id_p("/compiler/nat2"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(-1.23),id_p("/compiler/nat2"),compiler.type_check);
    PROCESS("/compiler/nat3 -> |/compiler/nat3?int{?}<=int(=>)[is(gt(0))]");
    //TYPE_SAVER(id_p("/compiler/nat3"),mmadt::Parser::singleton()->parse("/compiler/nat3?int{?}<=int(=>)[is(gt(0))]"));
    FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat3"),compiler.type_check);
    FOS_TEST_COMPILER_TRUE(jnt(-1),id_p("/compiler/nat3"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(1.23),id_p("/compiler/nat3"),compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(-1.23),id_p("/compiler/nat3"),compiler.type_check);
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_type_check_base_types); //
    FOS_RUN_TEST(test_type_check_derived_mono_types); //
  )
} // namespace fhatos

SETUP_AND_LOOP();