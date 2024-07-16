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
#ifndef fhatos_test_fhatos_hpp
#define fhatos_test_fhatos_hpp

#include <fhatos.hpp>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <unity.h>
#include FOS_PROCESS(scheduler.hpp)
#include <language/types.hpp>
#include <process/router/local_router.hpp>
#include <process/router/meta_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
#include <structure/kernel.hpp>
#include <util/options.hpp>
////////////////////////////////////////////////////////
//////////////////////// NATIVE ////////////////////////
////////////////////////////////////////////////////////
#ifdef NATIVE
namespace fhatos {
#ifndef FOS_LOG_TYPE
#define FOS_LOG_TYPE INFO
#endif

#ifndef FOS_TEST_ROUTERS
#ifdef NATIVE
#define FOS_TEST_ROUTERS LocalRouter::singleton(), MqttRouter::singleton(), MetaRouter::singleton()
#else
#define FOS_TEST_ROUTERS LocalRouter::singleton()
#endif
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
    try {                                                                                                              \
      Kernel::build()                                                                                                  \
          ->initialPrinter(Ansi<>::singleton())                                                                        \
          ->initialLogLevel(INFO)                                                                                      \
          ->withSplash(ANSI_ART)                                                                                       \
          ->withNote("Use !bØ!! for noobj")                                                                            \
          ->withNote("Use :help for console commands")                                                                 \
          ->onBoot(Scheduler::singleton("/sys/scheduler/"),                                                            \
                   {FOS_TEST_ROUTERS, Types::singleton("/sys/lang/type/"), Parser::singleton("/sys/lang/parser/")})    \
          ->loadModules({"/ext/process"}); /*->defaultOutput("/home/root/repl/")        */                             \
      UNITY_BEGIN();                                                                                                   \
      x;                                                                                                               \
      UNITY_END();                                                                                                     \
      /*Kernel::done("testing_barrier"); */                                                                            \
    } catch (std::exception & e) {                                                                                     \
      LOG(ERROR, "Failed test suite due to %s: %s\n", e.what(), STR(x));                                               \
    }                                                                                                                  \
  }
} // namespace fhatos

#define SETUP_AND_LOOP()                                                                                               \
  using namespace fhatos;                                                                                              \
  int main(int arg, char **argsv) { fhatos::RUN_UNITY_TESTS(); };                                                      \
  void setUp() {}                                                                                                      \
  void tearDown() {                                                                                                    \
    for (auto *router: List<Router *>({FOS_TEST_ROUTERS})) {                                                           \
      router->clear();                                                                                                 \
    }                                                                                                                  \
  }
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


template<typename OBJ = Obj>
static ptr<List<ptr<OBJ>>> FOS_TEST_RESULT(const Fluent &fluent, const bool printResult = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  ptr<List<Obj_p>> result = fluent.toList<Obj>();
  if (printResult) {
    int index = 0;
    for (const auto &obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       OTypes.toChars(obj->o_type()));
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
  FOS_TEST_MESSAGE("!yTesting less than!! : %s %s %s", objA->toString().c_str(),
                   test ? "<" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

template<typename T>
static const T *FOS_PRINT_OBJ(const T *obj) {
  FOS_TEST_MESSAGE("!yTesting!!: %s [otype:!y%s!!][itype:!y%s!!]", obj->toString().c_str(),
                   OTypes.toChars(obj->o_type()), ITypeDescriptions.toChars(obj->itype()));
  return obj;
}

template<typename T>
static const ptr<T> FOS_PRINT_OBJ(const ptr<T> obj) {
  FOS_PRINT_OBJ<T>(obj.get());
  return obj;
}

static void FOS_TEST_ERROR(const string &monoid) {
  try {
    Fluent(Parser::singleton()->tryParseObj(monoid).value()).iterate();
    TEST_ASSERT_TRUE_MESSAGE(false, ("No exception thrown in " + monoid).c_str());
  } catch (fError error) {
    LOG(INFO, "Expected !rexception thrown!!: %s\n", error.what());
    TEST_ASSERT_TRUE(true);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(const List<OBJ> &expected, const Fluent &fluent,
                              const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
                              const bool clearRouter = true) {
  const ptr<List<ptr<OBJ>>> result = FOS_TEST_RESULT<OBJ>(fluent);
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected.size(), result->size(), "Expected result size");
  for (const OBJ &obj: expected) {
    auto x = std::find_if(result->begin(), result->end(), [obj](const ptr<OBJ> element) {
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
    if (GLOBAL_OPTIONS->router<Router>()->retainSize() != -1) {
      TEST_ASSERT_EQUAL_INT_MESSAGE(
          expectedReferences.size(), GLOBAL_OPTIONS->router<Router>()->retainSize(),
          (string("Router retain message count: ") + GLOBAL_OPTIONS->router<Router>()->id()->toString()).c_str());
      for (const auto &[key, value]: expectedReferences) {
        const Obj temp = value;
        GLOBAL_OPTIONS->router<Router>()->subscribe(Subscription{
            .mailbox = nullptr,
            .source = ID(FOS_DEFAULT_SOURCE_ID),
            .pattern = key.uri_value(),
            .onRecv = [temp](const ptr<Message> &message) {
              TEST_ASSERT_TRUE_MESSAGE(temp == *message->payload, (string("Router retain message payload equality: ") +
                                                                   GLOBAL_OPTIONS->router<Router>()->id()->toString())
                                                                      .c_str());
            }});
      }
    }
  }
  if (clearRouter)
    GLOBAL_OPTIONS->router<Router>()->clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(const List<OBJ> &expected, const List<string> &monoids,
                              const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
                              const bool clearRouter = true) {
  const string &finalString = monoids.back();
  for (int i = 0; i < monoids.size() - 1; i++) {
    LOG(DEBUG, FOS_TAB_2 "!yPre-monoid!!: %s\n", monoids.at(i).c_str());
    Fluent(Parser::singleton()->tryParseObj(monoids.at(i)).value()).iterate();
  }
  LOG(DEBUG, "!gEnd monoid!!: %s\n", finalString.c_str());
  return FOS_CHECK_RESULTS<OBJ>(expected, Fluent(Parser::singleton()->tryParseObj(finalString).value()),
                                expectedReferences, clearRouter);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(const List<OBJ> &expected, const string &monoid,
                              const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
                              const bool clearRouter = false) {

  Option<Obj_p> parse = Parser::singleton()->tryParseObj(monoid);
  if (!parse.has_value())
    throw fError("Unable to parse: %s\n", monoid.c_str());
  return FOS_CHECK_RESULTS<OBJ>(expected, Fluent(parse.value()), expectedReferences, clearRouter);
}
#endif
