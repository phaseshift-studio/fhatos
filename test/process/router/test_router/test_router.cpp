#ifndef fhatos_test_router_hpp
#define fhatos_test_router_hpp

#include <test_fhatos.hpp>
//
#include <furi.hpp>
#include <process/actor/actor.hpp>
#include <process/router/local_router.hpp>
#include <process/router/meta_router.hpp>
#include <process/router/mqtt_router.hpp>

namespace fhatos::kernel {

void test_subscribe() {}

void test_publish() {
  TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS,
                    LocalRouter<>::singleton()->publish(Message{
                        .source = ID("a"),
                        .target = ID("b"),
                        .payload = {.type = STR, .data = (byte *)"test", .length=4},
                        .retain = TRANSIENT_MESSAGE}));
};

FOS_RUN_TESTS(                  //
    LocalRouter<>::singleton(); //
    FOS_RUN_TEST(test_publish); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
