/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/


#ifndef fhatos_test_fhatos_native_hpp
#define fhatos_test_fhatos_native_hpp
#ifdef NATIVE
// #define FOS_LOGGING DEBUG

#include <fhatos.hpp>
// #include <language/fluent.hpp>
#include <unity.h>

void setUp() {}
void tearDown() {}

#define FOS_TEST_PRINTER FOS_DEFAULT_PRINTER

#define SETUP_AND_LOOP()                                                                                               \
  int main(int arg, char **argsv) { fhatos::RUN_UNITY_TESTS(); };

#define FOS_PRINT_FLUENT(fluent)                                                                                       \
  FOS_TEST_MESSAGE("!yTesting!!: %s", (fluent).toString().c_str())                                                     \
  (fluent)

#define FOS_TEST_MESSAGE(format, ...)                                                                                  \
  if (FOS_LOGGING < ERROR) {                                                                                           \
    FOS_TEST_PRINTER::singleton()->printf("  !rline %i!!\t", __LINE__);                                                \
    FOS_TEST_PRINTER::singleton()->printf((format), ##__VA_ARGS__);                                                    \
    FOS_TEST_PRINTER::singleton()->printf("\n");                                                                       \
  }

#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                                                               \
  FOS_TEST_MESSAGE("!b%s!! =!r?!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                           \
  TEST_ASSERT_TRUE((x).equals(y));

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                                                           \
  FOS_TEST_MESSAGE("!b%s!! =!r/?!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                          \
  TEST_ASSERT_FALSE((x).equals(y))

#define FOS_TEST_ASSERT_EQUAL_CHAR_FURI(x, y) TEST_ASSERT_EQUAL_STRING((x), (y.toString().c_str()))

#define FOS_TEST_ASSERT_EXCEPTION(x)                                                                                   \
  try {                                                                                                                \
    x;                                                                                                                 \
    TEST_ASSERT(false);                                                                                                \
  } catch (fError & e) {                                                                                               \
    FOS_TEST_MESSAGE("!rAn expected error occurred!!: %s\n", e.what());                                                \
    TEST_ASSERT(true);                                                                                                 \
  }

#define FOS_TEST_OBJ_EQUAL(objA, objB)                                                                                 \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!yTesting equality!! : %s %s %s", (objA)->toString().c_str(),                                    \
                     test ? "==" : "!=", (objB)->toString().c_str());                                                  \
    if (!test)                                                                                                         \
      TEST_FAIL();                                                                                                     \
  }

#define FOS_TEST_OBJ_NOT_EQUAL(objA, objB)                                                                             \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!yTesting not equal!!: %s %s %s", (objA)->toString().c_str(),                                    \
                     test ? "==" : "!=", (objB)->toString().c_str());                                                  \
    if (test)                                                                                                          \
      TEST_FAIL();                                                                                                     \
  }

namespace fhatos {
#define FOS_RUN_TEST(x)                                                                                                \
  { RUN_TEST(x); }

#define FOS_RUN_TESTS(x)                                                                                               \
  void RUN_UNITY_TESTS() {                                                                                             \
    LOG(NONE, ANSI_ART);                                                                                               \
    UNITY_BEGIN();                                                                                                     \
    x;                                                                                                                 \
    UNITY_END();                                                                                                       \
  }
} // namespace fhatos

#endif
#endif
