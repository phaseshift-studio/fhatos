#ifndef fhatos_kernel__test_router_hpp
#define fhatos_kernel__test_router_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/router/local_router.hpp>
#include <kernel/process/router/meta_router.hpp>
#include <kernel/process/router/mqtt_router.hpp>
#include <unity.h>

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
