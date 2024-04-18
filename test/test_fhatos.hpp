#ifndef fhatos_kernel__test_fhatos_hpp
#define fhatos_kernel__test_fhatos_hpp

#include <unity.h>
//
#include <fhatos.hpp>
#include <kernel/util/ansi.hpp>

#define UNITY_BEGIN()                                                          \
  Serial.println(fhatos::kernel::ANSI_ART);                                    \
  UnityBegin(__FILE__)

#define SETUP_AND_LOOP()                                                       \
  void setup() {                                                               \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                         \
    delay(2000);                                                               \
    RUN_UNITY_TESTS();                                                         \
  }                                                                            \
  void loop() {}

#define RUN_TESTS(x)                                                           \
  void RUN_UNITY_TESTS() {                                                     \
    UNITY_BEGIN();                                                             \
    x;                                                                         \
    UNITY_END();                                                               \
  }

#define TEST_MESSAGE(format, ...)                                              \
  Serial.printf("  line %i", __LINE__);                                        \
  Serial.printf((format), ##__VA_ARGS__);                                      \
  Serial.println();

#define TEST_ASSERT_EQUAL_FURI(x, y)                                           \
  TEST_MESSAGE("\t%s =?= %s", (x).toString().c_str(), (y).toString().c_str()); \
  TEST_ASSERT_TRUE((x).equals(y));

#define TEST_ASSERT_NOT_EQUAL_FURI(x, y) TEST_ASSERT_FALSE((x).equals(y))

#define TEST_ASSERT_EQUAL_CHAR_FURI(x, y)                                      \
  TEST_ASSERT_EQUAL_STRING((x), (y.toString().c_str()))

#endif