#ifndef fhatos_kernel__test_esp32_scheduler_hpp
#define fhatos_kernel__test_esp32_scheduler_hpp

#include <test_fhatos.hpp>
//
#include <kernel/structure/structure.hpp>
#include <kernel/process/esp32/scheduler.hpp>

#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(coroutine.hpp)

#include <unity.h>

namespace fhatos::kernel {

void test_true() { TEST_ASSERT_TRUE(true); };

FOS_RUN_TESTS(               //
    FOS_RUN_TEST(test_true); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif