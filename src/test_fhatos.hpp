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

#ifndef fhatos_test_fhatos_hpp
#define fhatos_test_fhatos_hpp

#ifdef NATIVE
#include <test_fhatos_native.hpp>
#else
#include <unity.h>
//
#include <fhatos.hpp>
#include <language/fluent.hpp>

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

static fhatos::Fluent<> FOS_TEST(const fhatos::Fluent<> &fluent) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  return fluent;
}

template<typename _OBJ>
static fhatos::List<const _OBJ *> *FOS_TEST_RESULT(const fhatos::Fluent<> &fluent, const bool printResult = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  fhatos::List<const _OBJ *> *result = fluent.toList<_OBJ>();
  if (printResult) {
    int index = 0;
    for (const _OBJ *obj: *result) {
      FOS_TEST_MESSAGE("!g=%i!!>%s [!y%s!!]", index++, obj->toString().c_str(), fhatos::OTYPE_STR.at(obj->type()));
    }
  }
  return result;
}
#endif
