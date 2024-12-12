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

#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPE
#include <../test/test_fhatos.hpp>


namespace fhatos {
  using namespace mmadt;


  void test_mono_parsing() {
    TEST_ASSERT_EQUAL(OType::NOOBJ, OBJ_PARSER("noobj")->o_type());
    // bool
    FOS_TEST_OBJ_EQUAL(dool(true), OBJ_PARSER("true"));
    FOS_TEST_OBJ_EQUAL(dool(false), OBJ_PARSER("false"));
    FOS_TEST_OBJ_EQUAL(dool(true), OBJ_PARSER("bool[true]"));
    FOS_TEST_OBJ_EQUAL(dool(false), OBJ_PARSER("bool[false]"));
    // int
    FOS_TEST_OBJ_EQUAL(jnt(1), OBJ_PARSER("1"));
    FOS_TEST_OBJ_EQUAL(jnt(-10), OBJ_PARSER("-10"));
    FOS_TEST_OBJ_EQUAL(jnt(0), OBJ_PARSER("int[0]"));
    FOS_TEST_OBJ_EQUAL(jnt(-50), OBJ_PARSER("int?int<=int[-50]"));
    // real
    FOS_TEST_OBJ_EQUAL(real(1.0), OBJ_PARSER("1.0"));
    FOS_TEST_OBJ_EQUAL(real(-10.0), OBJ_PARSER("-10.0"));
    FOS_TEST_OBJ_EQUAL(real(0), OBJ_PARSER("real[0.0]"));
    // str
    FOS_TEST_OBJ_EQUAL(str("one.oh"), OBJ_PARSER("'one.oh'"));
    FOS_TEST_OBJ_EQUAL(str("negatIVE te  N"), OBJ_PARSER("'negatIVE te  N'"));
    FOS_TEST_OBJ_EQUAL(str(""), OBJ_PARSER("str['']"));
    FOS_TEST_OBJ_EQUAL(str("abc"), OBJ_PARSER("str['abc']"));
  }

  FOS_RUN_TESTS ( //
  FOS_RUN_TEST (test_mono_parsing); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
