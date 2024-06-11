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

#define SETUP_AND_LOOP()                                                                                               \
  void setup() {                                                                                                       \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                                                                 \
    delay(2000);                                                                                                       \
    fhatos::RUN_UNITY_TESTS();                                                                                         \
  }                                                                                                                    \
  void loop() {}

#define FOS_TEST_MESSAGE(format, ...)                                                                                  \
  FOS_TEST_PRINTER.printf("  !rline %i!!\t", __LINE__);                                                                \
  FOS_TEST_PRINTER.printf((format), ##__VA_ARGS__);                                                                    \
  FOS_TEST_PRINTER.println();

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
  } catch (fhatos::fError e) {                                                                                         \
    TEST_ASSERT(true);                                                                                                 \
  }

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
    LOG(NONE, ANSI_ART);                                                                                               \
    UNITY_BEGIN();                                                                                                     \
    uint32_t __test_freeSketch;                                                                                        \
    uint32_t __test_freeHeap;                                                                                          \
    x;                                                                                                                 \
    UNITY_END();                                                                                                       \
  }
} // namespace fhatos
#endif
#include "../_deps/unity-src/src/unity.h"

using namespace fhatos;

template<typename _OBJ = Obj>
static void FOS_CHECK_ARGS(const List<_OBJ *> &expectedArgs, const Inst *inst) {
  FOS_TEST_MESSAGE("!yTesting!! instruction: %s", inst->toString().c_str());
  TEST_ASSERT_EQUAL_INT(expectedArgs.size(), inst->args().size());
  for (int i = 0; i < expectedArgs.size(); i++) {
    bool test = *expectedArgs[i] == *inst->args()[i];
    if (!test) {
      FOS_TEST_MESSAGE("!r%s!! != !r%s!!", expectedArgs[i]->toString().c_str(), inst->args()[i]->toString().c_str());
      TEST_FAIL();
    }
  }
}

template<typename _OBJ>
static List<const _OBJ *> *FOS_TEST_RESULT(const Fluent<> &fluent, const bool printResult = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  List<const _OBJ *> *result = fluent.toList<_OBJ>();
  if (printResult) {
    int index = 0;
    for (const _OBJ *obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       fhatos::OTYPE_STR.at(obj->type()));
    }
  }
  return result;
}

static const void FOS_TEST_OBJ_EQUAL(const Obj *objA, const Obj *objB) {
  const bool test = *objA == *objB;
  FOS_TEST_MESSAGE("!bTesting equality!! : %s %s %s", objA->toString().c_str(),
                   test ? "==" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

static const void FOS_TEST_OBJ_NOT_EQUAL(const Obj *objA, const Obj *objB) {
  const bool test = *objA == *objB;
  FOS_TEST_MESSAGE("!bTesting not equal!!: %s %s %s", objA->toString().c_str(),
                   test ? "==" : "!=", objB->toString().c_str());
  if (test)
    TEST_FAIL();
}

template<typename T>
static const T *FOS_PRINT_OBJ(const T *obj) {
  FOS_TEST_MESSAGE("!yTesting!!: %s [!y%s!!]", obj->toString().c_str(), OTYPE_STR.at(obj->type()));
  return obj;
}

template<typename T>
static const ptr<T> FOS_PRINT_OBJ(const ptr<T> obj) {
  FOS_PRINT_OBJ<T>(obj.get());
  return obj;
}

template<typename _OBJ>
static void FOS_CHECK_RESULTS(const List<_OBJ> expected, const Fluent<> &fluent,
                              const Map<Uri, Obj *> expectedReferences = {}, const bool clearRouter = true) {
  const List<const _OBJ *> *result = FOS_TEST_RESULT<_OBJ>(fluent);
  TEST_ASSERT_EQUAL_INT(expected.size(), result->size());
  for (const _OBJ obj: expected) {
    auto x = std::find_if(result->begin(), result->end(), [obj](const _OBJ *element) { return obj == *element; });
    if (result->end() == x) {
      TEST_FAIL_MESSAGE(("Unable to find " + obj.toString()).c_str());
    }
  }
  if (!expectedReferences.empty()) {
    for (const auto &[key, value]: expectedReferences) {
      const Obj *temp = value;
      FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr, .source = ID("anon"), .pattern = key.value(), .onRecv = [temp](const Message &message) {
            TEST_ASSERT_TRUE(*temp == *message.payload->toObj());
          }});
    }
  }
  if (clearRouter)
    FOS_DEFAULT_ROUTER::singleton()->clear();
}
#endif
