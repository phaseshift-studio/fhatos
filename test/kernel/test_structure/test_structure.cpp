#ifndef fhatos_kernel__test_structure_hpp
#define fhatos_kernel__test_structure_hpp

#include <../test_fhatos.hpp>
//
#include <kernel/structure/structure.hpp>

using namespace fhatos::kernel;

void test_true() { TEST_ASSERT_TRUE(true); }

void test_furi_equals() {
  // TEST_ASSERT_TRUE(fURI("127.0.0.1").equals(fURI("127.0.0.1")));
}

void test_furi_match() {
  // TEST_ASSERT_TRUE(fURI("127.0.0.1").matches(fURI("127.0.0.1")));
  // TEST_ASSERT_TRUE(ID("127.0.0.1").matches(fURI("+")));
  // TEST_ASSERT_TRUE(ID("127.0.0.1").matches(fURI("#")));
  // TEST_ASSERT_TRUE(ID("127.0.0.1").matches(fURI("127.0.0.1/#")));

  ////
  // TEST_ASSERT_FALSE(ID("127.0.0.1").matches(fURI("127.0.0.2")));
  // TEST_ASSERT_FALSE(ID("127.0.0.1").matches(fURI("127.0.0.1/+")));
}

RUN_TESTS(
    //
    RUN_TEST(test_true);
    //
    RUN_TEST(test_furi_equals);
    //
    RUN_TEST(test_furi_match);
    //
);

SETUP_AND_LOOP()

#endif