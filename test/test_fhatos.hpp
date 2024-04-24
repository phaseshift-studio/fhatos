#ifndef fhatos_kernel__test_fhatos_hpp
#define fhatos_kernel__test_fhatos_hpp

#include <unity.h>
//
#include <fhatos.hpp>
#include <kernel/util/ansi.hpp>

#define SETUP_AND_LOOP()                                                       \
  void setup() {                                                               \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                         \
    delay(2000);                                                               \
    fhatos::kernel::RUN_UNITY_TESTS();                                         \
  }                                                                            \
  void loop() {}

#define FOS_TEST_MESSAGE(format, ...)                                          \
  Serial.printf("  line %i\t", __LINE__);                                      \
  Serial.printf((format), ##__VA_ARGS__);                                      \
  Serial.println();

#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                       \
  FOS_TEST_MESSAGE("%s =?= %s", (x).toString().c_str(),                        \
                   (y).toString().c_str());                                    \
  TEST_ASSERT_TRUE((x).equals(y));

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                   \
  FOS_TEST_MESSAGE("%s !=?= %s", (x).toString().c_str(),                       \
                   (y).toString().c_str());                                    \
  TEST_ASSERT_FALSE((x).equals(y))

#define FOS_TEST_ASSERT_EQUAL_CHAR_FURI(x, y)                                  \
  TEST_ASSERT_EQUAL_STRING((x), (y.toString().c_str()))

#define FOS_TEST_ASSERT_EXCEPTION(x)                                           \
  try {                                                                        \
    x;                                                                         \
    TEST_ASSERT(false);                                                        \
  } catch (fhatos::kernel::fError e) {                                         \
    TEST_ASSERT(true);                                                         \
  }

namespace fhatos::kernel {
#define FOS_RUN_TEST(x)                                                        \
  __test_freeSketch = ESP.getFreeSketchSpace();                                \
  __test_freeHeap = ESP.getFreeHeap();                                         \
  { RUN_TEST(x); }                                                             \
  UNITY_TEST_ASSERT_EQUAL_INT32(__test_freeSketch, ESP.getFreeSketchSpace(),   \
                                __LINE__, "Memory leak in sketch space.");     \
  UNITY_TEST_ASSERT_EQUAL_INT32(__test_freeHeap, ESP.getFreeHeap(), __LINE__,  \
                                "Memory leak in heap.");

#define FOS_RUN_TESTS(x)                                                       \
  void RUN_UNITY_TESTS() {                                                     \
    LOG(NONE, ANSI_ART);                                                       \
    UNITY_BEGIN();                                                             \
    uint32_t __test_freeSketch;                                                \
    uint32_t __test_freeHeap;                                                  \
    x;                                                                         \
    UNITY_END();                                                               \
  }
} // namespace fhatos::kernel

#endif