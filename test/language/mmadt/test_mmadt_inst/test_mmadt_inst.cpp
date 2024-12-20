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

  void test_plus_inst() {
    FOS_TEST_OBJ_EQUAL(objs({jnt(11),jnt(12),jnt(12),jnt(77)}), PROCESS_ALL("{1,2,2,67}.plus(10)"));
    FOS_TEST_OBJ_EQUAL(jnt(246), PROCESS("1.plus(245)"));
  }

  void test_count_inst() {
    FOS_TEST_OBJ_EQUAL(jnt(4), PROCESS("{1,2,2,67}.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(1), PROCESS("35.count()"));
    FOS_TEST_OBJ_EQUAL(jnt(5), PROCESS("1-<[_,_,_,_,_]>-.count()"));
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_plus_inst); //
    FOS_RUN_TEST(test_count_inst); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
