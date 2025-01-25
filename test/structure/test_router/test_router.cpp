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

#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /router/#
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"
#include "../../../src/util/string_helper.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_router_config() {

  }

  void test_router_attach_detach() {
    FOS_TEST_ERROR("/temp/abc -> 12");
    PROCESS("/router/a -> /fos/heap[[pattern=>/temp/#]]");
    int size = Router::singleton()->rec_get("structure")->lst_value()->size();
    FOS_TEST_ERROR("/temp/abc -> 12");
    PROCESS("/sys/router/:attach(*/router/a)");
    TEST_ASSERT_EQUAL_INT(size+1,Router::singleton()->rec_get("structure")->lst_value()->size());
    FOS_TEST_OBJ_EQUAL(jnt(12), PROCESS("/temp/abc -> 12"));
    FOS_TEST_OBJ_EQUAL(jnt(12), PROCESS("*/temp/abc"));
    PROCESS("/sys/router/:detach(/temp/#)");
    Router::singleton()->loop(); // ensure detached structure is removed from router's table
    TEST_ASSERT_EQUAL_INT(size,Router::singleton()->rec_get("structure")->lst_value()->size());
    FOS_TEST_ERROR("/temp/abc -> 12");
  }

  void test_retain_write() {
    FOS_TEST_ERROR("/r/abc -> 'blah'");
    FOS_TEST_ERROR("/abc -> 'blah'");
    for(int i=0; i<100;i=i+5) {
      FOS_TEST_OBJ_EQUAL(jnt(i),PROCESS(string("/router/abc -> ") + to_string(i)));
    }
    FOS_TEST_OBJ_EQUAL(jnt(95), PROCESS("*/router/abc"));
  }

  void test_transient_write() {
    PROCESS("/router/abc1 -> |(plus(10).to(/router/bcd))");
    PROCESS("/router/abc2 -> |at(/router/cde)");
    for(int i=0; i<100;i=i+5) {
      PROCESS(string("/router/abc1 --> ") + to_string(i));
      PROCESS(string("/router/abc2 --> ") + to_string(i));
      FOS_TEST_OBJ_EQUAL(jnt(i+10), PROCESS("*/router/bcd"));
      FOS_TEST_OBJ_EQUAL(jnt(i), PROCESS("*/router/cde"));
    }
    FOS_TEST_OBJ_EQUAL(jnt(105), PROCESS("*/router/bcd"));
    FOS_TEST_OBJ_EQUAL(jnt(95), PROCESS("*/router/cde"));
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_router_config); //
    FOS_RUN_TEST(test_router_attach_detach); //
    FOS_RUN_TEST(test_retain_write); //
    FOS_RUN_TEST(test_transient_write); //
  )

}
SETUP_AND_LOOP();