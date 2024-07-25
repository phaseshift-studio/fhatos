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
#pragma once
#ifndef fhatos_test_fhatos_fast_hpp
#define fhatos_test_fhatos_fast_hpp

#include <fhatos.hpp>
#include <unity.h>
#include <util/ansi.hpp>
////////////////////////////////////////////////////////
//////////////////////// NATIVE ////////////////////////
////////////////////////////////////////////////////////
#ifdef NATIVE
namespace fhatos {
#ifndef FOS_LOG_TYPE
#define FOS_LOG_TYPE INFO
#endif
#define FOS_RUN_TEST(x)                                                                                                \
  {                                                                                                                    \
    try {                                                                                                              \
      RUN_TEST(x);                                                                                                     \
    } catch (const std::exception &e) {                                                                                \
      LOG(ERROR, "Failed test due to %s: %s\n", e.what(), STR(x));                                                     \
      throw;                                                                                                           \
    }                                                                                                                  \
  }

#define FOS_RUN_TESTS(x)                                                                                               \
  void RUN_UNITY_TESTS() {                                                                                             \
    GLOBAL_OPTIONS->PRINTING = Ansi<>::singleton();                                                                    \
    try {                                                                                                              \
      UNITY_BEGIN();                                                                                                   \
      x;                                                                                                               \
      UNITY_END();                                                                                                     \
    } catch (std::exception & e) {                                                                                     \
      LOG(ERROR, "Failed test suite due to %s: %s\n", e.what(), STR(x));                                               \
      TEST_FAIL();                                                                                                     \
    }                                                                                                                  \
  }

} // namespace fhatos
#define SETUP_AND_LOOP()                                                                                               \
  using namespace fhatos;                                                                                              \
  int main(int, char **) { RUN_UNITY_TESTS(); };                                                                       \
  void setUp() {}                                                                                                      \
  void tearDown() {}
#else
/////////////////////////////////////////////////////
//////////////////////// ESP ////////////////////////
/////////////////////////////////////////////////////


#define SETUP_AND_LOOP()                                                                                               \
  void setup() {                                                                                                       \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                                                                 \
    delay(2000);                                                                                                       \
    fhatos::RUN_UNITY_TESTS();                                                                                         \
  }                                                                                                                    \
  void loop() {}

namespace fhatos {
#define FOS_RUN_TEST(x)                                                                                                \
  __test_freeSketch = ESP.getFreeSketchSpace();                                                                        \
  __test_freeHeap = ESP.getFreeHeap();                                                                                 \
  { RUN_TEST(x); }                                                                                                     \
  TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(__test_freeSketch, ESP.getFreeSketchSpace(),                                 \
                                          "Memory leak in sketch space.");                                             \
  TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(__test_freeHeap, ESP.getFreeHeap(), "Memory leak in heap.");

#define FOS_RUN_TESTS(x)                                                                                               \
  void RUN_UNITY_TESTS() {                                                                                             \
    GLOBAL_OPTIONS->LOGGING = LOG_TYPE::TRACE;                                                                         \
    GLOBAL_OPTIONS->PRINTING = Ansi<CPrinter>::singleton();                                                            \
    GLOBAL_OPTIONS->ROUTING = LocalRouter::singleton();                                                                \
    LOG(NONE, ANSI_ART);                                                                                               \
    Scheduler::singleton()->onBoot({LocalRouter::singleton(), Parser::singleton(), Types::singleton()});               \
    Types::singleton()->loadExt("/ext/process");                                                                       \
    UNITY_BEGIN();                                                                                                     \
    uint32_t __test_freeSketch;                                                                                        \
    uint32_t __test_freeHeap;                                                                                          \
    x;                                                                                                                 \
    UNITY_END();                                                                                                       \
  }
} // namespace fhatos

#endif

using namespace fhatos;

#define FOS_TEST_PRINTER FOS_DEFAULT_PRINTER
#define FOS_PRINT_FLUENT(fluent)                                                                                       \
  FOS_TEST_MESSAGE("!yTesting!!: %s", (fluent).toString().c_str())                                                     \
  (fluent)

#define FOS_TEST_MESSAGE(format, ...)                                                                                  \
  if (FOS_LOGGING < ERROR) {                                                                                           \
    GLOBAL_OPTIONS->printer<>()->printf("  !rline %i!!\t", __LINE__);                                                  \
    GLOBAL_OPTIONS->printer<>()->printf((format), ##__VA_ARGS__);                                                      \
    GLOBAL_OPTIONS->printer<>()->printf("\n");                                                                         \
  }

#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                                                               \
  FOS_TEST_MESSAGE("!b%s!! =!r?!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                           \
  TEST_ASSERT_TRUE((x).equals(y));

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                                                           \
  FOS_TEST_MESSAGE("!b%s!! =!r/?!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                          \
  TEST_ASSERT_FALSE((x).equals(y))

#define FOS_TEST_ASSERT_EQUAL_CHAR_FURI(x, y) TEST_ASSERT_EQUAL_STRING((x), (y.toString().c_str()))

#define FOS_TEST_ASSERT_EXCEPTION(x, s)                                                                                \
  try {                                                                                                                \
    if ((s)) {                                                                                                         \
      Fluent(Parser::tryParseObj((STR(x))).value()).iterate();                                                         \
    } else {                                                                                                           \
      x;                                                                                                               \
    }                                                                                                                  \
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
