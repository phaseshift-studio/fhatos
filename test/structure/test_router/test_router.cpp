#ifndef fhatos_test_router_hpp
#define fhatos_test_router_hpp

#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /compiler/#
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"
#include "../../../src/util/string_helper.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_router_config() {

    }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_router_config); //
  )

}
SETUP_AND_LOOP();
#endif