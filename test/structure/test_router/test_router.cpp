#ifndef fhatos_test_router_hpp
#define fhatos_test_router_hpp

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

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_router_config); //
    FOS_RUN_TEST(test_router_attach_detach); //
  )

}
SETUP_AND_LOOP();
#endif