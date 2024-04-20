#ifndef fhatos_kernel__test_local_broker_hpp
#define fhatos_kernel__test_local_broker_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/broker/local_broker/local_broker.hpp>
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_true() { TEST_ASSERT_TRUE(true); }

void test_publish() {
  LocalBroker<StringMessage> *broker = LocalBroker<StringMessage>::singleton();

  TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS,
                    broker->publish(StringMessage{.source = ID("a"),
                                                  .target = ID("b"),
                                                  .payload = String("test"),
                                                  .retain = false}));
};

RUN_TESTS(                  //
    RUN_TEST(test_true);    //
    RUN_TEST(test_publish); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
