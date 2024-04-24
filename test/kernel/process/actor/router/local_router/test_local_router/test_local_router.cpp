#ifndef fhatos_kernel__test_local_router_hpp
#define fhatos_kernel__test_local_router_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/router/local_router/local_router.hpp>
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

LocalRouter<StringMessage> *router = LocalRouter<StringMessage>::singleton();

void test_subscribe() {}

void test_publish() {
  TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS,
                    router->publish(StringMessage{.source = ID("a"),
                                                  .target = ID("b"),
                                                  .payload = String("test"),
                                                  .retain = false}));
};

FOS_RUN_TESTS(                  //
    FOS_RUN_TEST(test_publish); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
