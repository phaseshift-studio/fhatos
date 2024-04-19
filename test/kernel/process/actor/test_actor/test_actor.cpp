#ifndef fhatos_kernel__test_actor_hpp
#define fhatos_kernel__test_actor_hpp

#include <../../../test_fhatos.hpp>
//
#include <unity.h>
#include <kernel/process/actor/actor.hpp>
#include <kernel/structure/structure.hpp>

using namespace fhatos::kernel;

void test_true() { TEST_ASSERT_TRUE(true); }

RUN_TESTS(               //
    RUN_TEST(test_true); //
);

SETUP_AND_LOOP()

#endif
