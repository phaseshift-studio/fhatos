#ifndef fhatos_kernel__test_actor_hpp
#define fhatos_kernel__test_actor_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/broker/local_broker/local_broker.hpp>
#include <kernel/process/task/esp32/scheduler.hpp>
#include <kernel/process/task/esp32/thread.hpp>
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_true() {
  Actor<Thread, StringMessage, LocalBroker<StringMessage>, String> actor =
      Actor<Thread, StringMessage, LocalBroker<StringMessage>, String>(
          ID("actor@fatos.org"));
  // actor.publish()
}

RUN_TESTS(               //
    RUN_TEST(test_true); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
