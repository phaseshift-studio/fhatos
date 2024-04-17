#ifndef fhatos_kernel__test_structure_hpp
#define fhatos_kernel__test_structure_hpp

#include <unity.h>
//
#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

using namespace fhatos::kernel;

void test_true() { TEST_ASSERT_TRUE(true); }

void RUN_UNITY_TESTS() {
  UNITY_BEGIN();
  RUN_TEST(test_true);
  UNITY_END();
}

void setup() {
  Serial.begin(FOS_SERIAL_BAUDRATE);
  delay(2000);
  RUN_UNITY_TESTS();
}

void loop() {}

#endif