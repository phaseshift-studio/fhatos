#ifndef fhatos_test_fhatos_hpp
#define fhatos_test_fhatos_hpp

#ifdef NATIVE
#include <test_fhatos_native.hpp>
#else
#include <unity.h>
//
#include <fhatos.hpp>

static fhatos::Ansi<HardwareSerial> ansi(&::Serial);
#define FOS_TEST_PRINTER ansi // Serial

#define SETUP_AND_LOOP()                                                       \
  void setup() {                                                               \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                         \
    delay(2000);                                                               \
    fhatos::RUN_UNITY_TESTS();                                         \
  }                                                                            \
  void loop() {}

#define FOS_TEST_MESSAGE(format, ...)                                          \
  FOS_TEST_PRINTER.printf("  !rline %i!!\t", __LINE__);                        \
  FOS_TEST_PRINTER.printf((format), ##__VA_ARGS__);                            \
  FOS_TEST_PRINTER.println();

#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                       \
  FOS_TEST_MESSAGE("!b%s!! =!r?!!= !b%s!!", (x).toString().c_str(),            \
                   (y).toString().c_str());                                    \
  TEST_ASSERT_TRUE((x).equals(y));

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                   \
  FOS_TEST_MESSAGE("!b%s!! =!r/?!!= !b%s!!", (x).toString().c_str(),           \
                   (y).toString().c_str());                                    \
  TEST_ASSERT_FALSE((x).equals(y))

#define FOS_TEST_ASSERT_EQUAL_CHAR_FURI(x, y)                                  \
  TEST_ASSERT_EQUAL_STRING((x), (y.toString().c_str()))

#define FOS_TEST_ASSERT_EXCEPTION(x)                                           \
  try {                                                                        \
    x;                                                                         \
    TEST_ASSERT(false);                                                        \
  } catch (fhatos::fError e) {                                         \
    TEST_ASSERT(true);                                                         \
  }

namespace fhatos {
#define FOS_RUN_TEST(x)                                                        \
  __test_freeSketch = ESP.getFreeSketchSpace();                                \
  __test_freeHeap = ESP.getFreeHeap();                                         \
  { RUN_TEST(x); }                                                             \
  TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(__test_freeSketch,                   \
                                          ESP.getFreeSketchSpace(),            \
                                          "Memory leak in sketch space.");     \
  TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(__test_freeHeap, ESP.getFreeHeap(),  \
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
#endif
