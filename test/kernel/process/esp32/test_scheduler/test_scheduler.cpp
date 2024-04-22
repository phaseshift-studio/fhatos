#ifndef fhatos_kernel__test_esp32_scheduler_hpp
#define fhatos_kernel__test_esp32_scheduler_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/esp32/scheduler.hpp>
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_true() { TEST_ASSERT_TRUE(true); };

RUN_TESTS(               //
    RUN_TEST(test_true); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif