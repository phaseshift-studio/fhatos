#ifndef fhatos_test_router_hpp
#define fhatos_test_router_hpp

#include <test_fhatos.hpp>
//
#include <process/actor/actor.hpp>
#include <process/router/local_router.hpp>
#include <language/binary_obj.hpp>

namespace fhatos {

  void test_subscribe() {
  }

  void test_publish() {
    TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS,
                      LocalRouter<>::singleton()->publish(Message{
                        .source = ID("a"),
                        .target = ID("b"),
                        .payload = new BinaryObj<>{ STR, (fbyte *)"test", 4},
                        .retain = TRANSIENT_MESSAGE}));
  };

  FOS_RUN_TESTS( //
      LocalRouter<>::singleton(); //
      FOS_RUN_TEST(test_publish); //
      );

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
