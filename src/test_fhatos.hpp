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
#include <fhatos.hpp>
#include <language/fluent.hpp>
#include <language/obj.hpp>
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

using namespace fhatos;

static void FOS_CHECK_ARGS(const List<Obj_p> &expectedArgs, const Inst_p &inst) {
  FOS_TEST_MESSAGE("!yTesting!! instruction: %s", inst->toString().c_str());
  TEST_ASSERT_EQUAL_INT(expectedArgs.size(), inst->inst_args().size());
  for (int i = 0; i < expectedArgs.size(); i++) {
    bool test = *expectedArgs[i] == *(inst->inst_arg(i));
    if (!test) {
      FOS_TEST_MESSAGE("!r%s!! != !r%s!!", expectedArgs[i]->toString().c_str(), inst->inst_arg(i)->toString().c_str());
      TEST_FAIL();
    }
  }
}

template<typename _OBJ = Obj>
static ptr<List<ptr<_OBJ>>> FOS_TEST_RESULT(const Fluent<> &fluent, const bool printResult = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  ptr<List<Obj_p>> result = fluent.toList<Obj>();
  if (printResult) {
    int index = 0;
    for (const auto &obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       OTYPE_STR.at(obj->o_range()));
    }
  }
  return result;
}

template<typename OBJ = Obj>
static const void FOS_TEST_OBJ_GT(const ptr<OBJ> objA, const ptr<OBJ> objB) {
  const bool test = *objA > *objB;
  FOS_TEST_MESSAGE("!yTesting greater than!! : %s %s %s", objA->toString().c_str(),
                   test ? ">" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

template<typename OBJ = Obj>
static const void FOS_TEST_OBJ_LT(const ptr<OBJ> objA, const ptr<OBJ> objB) {
  const bool test = *objA < *objB;
  FOS_TEST_MESSAGE("!yTesting greater than!! : %s %s %s", objA->toString().c_str(),
                   test ? "<" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

template<typename T>
static const T *FOS_PRINT_OBJ(const T *obj) {
  FOS_TEST_MESSAGE("!yTesting!!: %s [id:!yN/A!!][stype:!y%s!!][utype:!y%s!!]", obj->toString().c_str(),
                   /*obj->id().toString().c_str(),*/ OTYPE_STR.at(obj->o_range()), obj->id()->toString().c_str());
  return obj;
}

template<typename T>
static const ptr<T> FOS_PRINT_OBJ(const ptr<T> obj) {
  FOS_PRINT_OBJ<T>(obj.get());
  return obj;
}

template<typename _OBJ = Obj>
static void FOS_CHECK_RESULTS(const List<_OBJ> &expected, const Fluent<> &fluent,
                              const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
                              const bool clearRouter = true) {
  const ptr<List<ptr<_OBJ>>> result = FOS_TEST_RESULT<_OBJ>(fluent);
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected.size(), result->size(), "Expected vs. actual result size");
  for (const _OBJ &obj: expected) {
    auto x = std::find_if(result->begin(), result->end(), [obj](const ptr<_OBJ> element) {
      if (obj.isReal()) {
        return obj.real_value() + 0.01f > element->real_value() && obj.real_value() - 0.01f < element->real_value();
      } else
        return obj == *element;
    });
    if (result->end() == x) {
      TEST_FAIL_MESSAGE(("Unable to find " + obj.toString()).c_str());
    }
  }
  if (!expectedReferences.empty()) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(expectedReferences.size(), FOS_DEFAULT_ROUTER::singleton()->retainSize(),
                                  "Expected vs. actual router retain message size");
    for (const auto &[key, value]: expectedReferences) {
      const Obj temp = value;
      FOS_DEFAULT_ROUTER::singleton()->subscribe(
          Subscription{.mailbox = nullptr,
                       .source = ID("anon"),
                       .pattern = key.uri_value(),
                       .onRecv = [temp](const ptr<Message> &message) { TEST_ASSERT_TRUE(temp == *message->payload); }});
    }
  }
  if (clearRouter)
    FOS_DEFAULT_ROUTER::singleton()->clear();
}
#endif
