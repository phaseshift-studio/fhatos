#ifdef NATIVE
#ifndef fhatos_test_fhatos_native_hpp
#define fhatos_test_fhatos_native_hpp

#include <unity.h>
#include <stdio.h>
#include <fhatos.hpp>

static fhatos::Ansi<fhatos::CPrinter> ansi(new fhatos::CPrinter());
#define FOS_TEST_PRINTER ansi

#define SETUP_AND_LOOP()                                                       \
int main(int arg, char **argsv) {           \
fhatos::RUN_UNITY_TESTS();                                              \
};

#define FOS_TEST_MESSAGE(format, ...)                                          \
  FOS_TEST_PRINTER.printf("  !rline %i!!\t", __LINE__);                        \
  FOS_TEST_PRINTER.printf((format), ##__VA_ARGS__);                            \
  FOS_TEST_PRINTER.printf("\n");

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
  } catch (fhatos::kernel::fError e) {                                         \
    TEST_ASSERT(true);                                                         \
  }

namespace fhatos {
#define FOS_RUN_TEST(x)                                                        \
  { RUN_TEST(x); }

#define FOS_RUN_TESTS(x)                                                       \
  void RUN_UNITY_TESTS() {                                                     \
  LOG(NONE, ANSI_ART);                                                         \
    UNITY_BEGIN();                                                             \
    x;                                                                         \
    UNITY_END();                                                               \
  }
} // namespace fhatos

#endif
#endif
