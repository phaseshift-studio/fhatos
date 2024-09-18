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

#ifndef FOS_LOGGING
#define FOS_LOGGING DEBUG
#endif

#include <fhatos.hpp>
#include <../build/_deps/unity-src/src/unity.h>
#include <unity.h>
#include <model/model.hpp>
#define FOS_DEPLOY_PRINTER_2                              \
  Options::singleton()->printer<>(Ansi<>::singleton());   \
  Options::singleton()->log_level(FOS_LOGGING);
#ifdef FOS_DEPLOY_PROCESSOR
#include <language/processor.hpp>
#define FOS_DEPLOY_PROCESSOR_2 load_processor();
#else
#define FOS_DEPLOY_PROCESSOR_2 ;
#endif

#ifdef FOS_DEPLOY_SCHEDULER
#include FOS_PROCESS(scheduler.hpp)
#define FOS_DEPLOY_SCHEDULER_2  Options::singleton()->scheduler<Scheduler>(Scheduler::singleton());
#else
#define FOS_DEPLOY_SCHEDULER_2 ;
#endif
#ifdef FOS_DEPLOY_ROUTER
#include <structure/router.hpp>
#include <language/fluent.hpp>
#define FOS_DEPLOY_ROUTER_2 Options::singleton()->router<Router>(Router::singleton());
#else
#define FOS_DEPLOY_ROUTER_2 ;
#endif
#ifdef FOS_DEPLOY_PARSER
#include <language/parser.hpp>
#include <language/fluent.hpp>
#define FOS_DEPLOY_PARSER_2  Model::deploy(Parser::singleton());
#else
#define FOS_DEPLOY_PARSER_2 ;
#endif
#ifdef FOS_DEPLOY_TYPES
#include <language/types.hpp>
#define FOS_DEPLOY_TYPES_2 Model::deploy(Types::singleton());
#else
#define FOS_DEPLOY_TYPES_2 ;
#endif
#ifdef FOS_DEPLOY_SHARED_MEMORY
#include <model/shared_memory.hpp>
#define FOS_DEPLOY_SHARED_MEMORY_2 Model::deploy(SharedMemory::create(ID("/memory/shared"), Pattern((0 ==strcmp("",STR(FOS_DEPLOY_SHARED_MEMORY))) ? "+" : STR(FOS_DEPLOY_SHARED_MEMORY))));
#else
#define FOS_DEPLOY_SHARED_MEMORY_2 ;
#endif
#ifdef FOS_DEPLOY_FILE_SYSTEM
#include FOS_FILE_SYSTEM(fs.hpp)
#define FOS_DEPLOY_FILE_SYSTEM_2 Model::deploy(FileSystem::create(ID("/io/fs/")));
#else
#define FOS_DEPLOY_FILE_SYSTEM_2 ;
#endif
#define FOS_STOP_ON_BOOT ;

////////////////////////////////////////////////////////
//////////////////////// NATIVE ////////////////////////
////////////////////////////////////////////////////////
namespace fhatos {
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
      FOS_DEPLOY_PRINTER_2                                                                                             \
      FOS_DEPLOY_PROCESSOR_2                                                                                           \
      FOS_DEPLOY_SCHEDULER_2                                                                                           \
      FOS_DEPLOY_ROUTER_2                                                                                              \
      FOS_DEPLOY_PARSER_2                                                                                              \
      FOS_DEPLOY_TYPES_2                                                                                               \
      FOS_DEPLOY_SHARED_MEMORY_2                                                                                       \
      FOS_DEPLOY_FILE_SYSTEM_2                                                                                         \
      UNITY_BEGIN();                                                                                                   \
      /*uint32_t __test_freeSketch;                                                                                    \
      uint32_t __test_freeHeap;  */                                                                                    \
      x;                                                                                                               \
      UNITY_END();                                                                                                     \
    } catch (const std::exception &e) {                                                                                \
      LOG(ERROR, "Failed test suite due to %s: %s\n", e.what(), STR(x));                                               \
      TEST_FAIL();                                                                                                     \
    }                                                                                                                  \
  }
} // namespace fhatos

#ifdef NATIVE
#define SETUP_AND_LOOP_2 int main(int, char **) {
#else
#define SETUP_AND_LOOP_2                                                                                               \
  void setup() {                                                                                                       \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                                                                 \
    delay(2000);        
#endif

#define SETUP_AND_LOOP()                                                                                               \
  using namespace fhatos;                                                                                              \
    SETUP_AND_LOOP_2                                                                                                   \
    RUN_UNITY_TESTS();                                                                                                 \
  };

void loop() {
}

void setUp() {
}

void tearDown() { FOS_STOP_ON_BOOT; }


//////////////////////////////////////
/////// TEST UTILITY FUNCTIONS /////// 
//////////////////////////////////////

using namespace fhatos;

#define FOS_PRINT_FLUENT(fluent)                                                                                       \
  FOS_TEST_MESSAGE("!yTesting!!: %s", (fluent).toString().c_str())                                                     \
  (fluent)

#define FOS_TEST_MESSAGE(format, ...)                                                                                  \
  if (FOS_LOGGING < ERROR) {                                                                                           \
    printer<>()->printf("  !rline %i!!\t", __LINE__);                                                                  \
    printer<>()->printf((format), ##__VA_ARGS__);                                                                      \
    printer<>()->println();                                                                                            \
  }

#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                                                               \
  FOS_TEST_MESSAGE("<!b%s!!> =!r?!!= <!b%s!!> (%i !rchar_length!! %i) (%i !rpath_length!! %i)",                        \
                   (x).toString().c_str(), (y).toString().c_str(), (x).toString().length(), (y).toString().length(),   \
                   (x).path_length(), (y).path_length());                                                              \
  TEST_ASSERT_TRUE_MESSAGE((x).equals(y),"Not equals()");                                                              \
  TEST_ASSERT_TRUE_MESSAGE((x) == (y), "Not ==");                                                                      \
  TEST_ASSERT_EQUAL_STRING((x).toString().c_str(), (y).toString().c_str());

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                                                           \
  FOS_TEST_MESSAGE("<!b%s!!> =!r/?!!= <!b%s!!> (%i !rchar_length!! %i) (%i !rpath_length!! %i)",                       \
                   (x).toString().c_str(), (y).toString().c_str(), (x).toString().length(), (y).toString().length(),   \
                   (x).path_length(), (y).path_length());                                                              \
  TEST_ASSERT_FALSE((x).equals(y));                                                                                    \
  TEST_ASSERT_TRUE((x) != (y));                                                                                        \
  TEST_ASSERT_TRUE((x).toString() != (y).toString());

#define FOS_TEST_ASSERT_EQUAL_CHAR_FURI(x, y) TEST_ASSERT_EQUAL_STRING((x), (y).toString().c_str())

#define FOS_TEST_ASSERT_MATCH_FURI(x, y)                                                                               \
  FOS_TEST_MESSAGE("!b%s!! =!r~!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                           \
  TEST_ASSERT_TRUE((x).matches(y));

#define FOS_TEST_ASSERT_NOT_MATCH_FURI(x, y)                                                                           \
  FOS_TEST_MESSAGE("!b%s!! =!r/~!!= !b%s!!", (x).toString().c_str(), (y).toString().c_str());                          \
  TEST_ASSERT_FALSE((x).matches(y));

#define FOS_TEST_EXCEPTION_CXX(x)                                                                                      \
  try {                                                                                                                \
    (x);                                                                                                               \
    TEST_ASSERT(false);                                                                                                \
  } catch (const fError &e) {                                                                                          \
    FOS_TEST_MESSAGE("!rAn expected error occurred!!: %s", e.what());                                                  \
    TEST_ASSERT(true);                                                                                                 \
  }

#ifdef FOS_DEPLOY_PARSER
#define FOS_TEST_ASSERT_EXCEPTION(x, s)                                                                                \
  try {                                                                                                                \
    if ((s)) {                                                                                                         \
      Fluent(Parser::tryParseObj((STR(x))).value()).iterate();                                                         \
    } else {                                                                                                           \
      x;                                                                                                               \
    }                                                                                                                  \
    TEST_ASSERT(false);                                                                                                \
  } catch (const fError &e) {                                                                                          \
    FOS_TEST_MESSAGE("!rAn expected error occurred!!: %s", e.what());                                                  \
    TEST_ASSERT(true);                                                                                                 \
  }
#endif

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

#ifdef FOS_DEPLOY_PARSER
template<typename OBJ = Obj>
static ptr<List<ptr<OBJ>>> FOS_TEST_RESULT(const Fluent &fluent, const bool printResult = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", fluent.toString().c_str());
  ptr<List<Obj_p>> result = fluent.toList<Obj>();
  if (printResult) {
    int index = 0;
    for (const auto &obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       OTypes.to_chars(obj->o_type()).c_str());
    }
  }
  return result;
}
#endif

template<typename OBJ = Obj>
static void FOS_TEST_OBJ_GT(const ptr<OBJ> objA, const ptr<OBJ> objB) {
  const bool test = *objA > *objB;
  FOS_TEST_MESSAGE("!yTesting greater than!! : %s %s %s", objA->toString().c_str(),
                   test ? ">" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

template<typename OBJ = Obj>
static void FOS_TEST_OBJ_LT(const ptr<OBJ> objA, const ptr<OBJ> objB) {
  const bool test = *objA < *objB;
  FOS_TEST_MESSAGE("!yTesting less than!! : %s %s %s", objA->toString().c_str(),
                   test ? "<" : "!=", objB->toString().c_str());
  if (!test)
    TEST_FAIL();
}

template<typename T>
static const T *FOS_PRINT_OBJ(const T *obj) {
  FOS_TEST_MESSAGE("!yTesting!!: %s [otype:!y%s!!][itype:!y%s!!]", obj->toString().c_str(),
                   OTypes.to_chars(obj->o_type()).c_str(), ITypeDescriptions.to_chars(obj->itype()).c_str());
  return obj;
}

template<typename T>
static const ptr<T> FOS_PRINT_OBJ(const ptr<T> obj) {
  FOS_PRINT_OBJ<T>(obj.get());
  return obj;
}

#ifdef FOS_DEPLOY_PARSER
[[maybe_unused]] static void FOS_TEST_ERROR(const string &monoid) {
  try {
    Fluent(Parser::singleton()->try_parse_obj(monoid).value()).iterate();
    TEST_ASSERT_TRUE_MESSAGE(false, ("No exception thrown in " + monoid).c_str());
  } catch (const fError &error) {
    LOG(INFO, "Expected !rexception thrown!!: %s\n", error.what());
    TEST_ASSERT_TRUE(true);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_ROUTER
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(
  const List<OBJ> &expected, const Fluent &fluent,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  [[maybe_unused]] const bool clearRouter = true) {
  const ptr<List<ptr<OBJ>>> result = FOS_TEST_RESULT<OBJ>(fluent);
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected.size(), result->size(), "Expected result size");
  for (const OBJ &obj: expected) {
    auto x = std::find_if(result->begin(), result->end(), [obj](const ptr<OBJ> element) {
      if (obj.is_real()) {
        return obj.real_value() + 0.01f > element->real_value() && obj.real_value() - 0.01f < element->real_value();
      } else
        return obj == *element;
    });
    if (result->end() == x) {
      TEST_FAIL_MESSAGE(("Unable to find " + obj.toString()).c_str());
    }
  }
  if (!expectedReferences.empty()) {
    /* TEST_ASSERT_EQUAL_INT_MESSAGE(
         expectedReferences.size(), router()->retainSize(),
         (string("Router retain message count: ") + router()->pattern()->toString()).c_str());*/
    for (const auto &[key, value]: expectedReferences) {
      const Obj temp = value;
      router()->route_subscription(share<Subscription>(
        Subscription{
          .source = ID("fhatty"),
          .pattern = key.uri_value(),
          .on_recv = Insts::to_bcode([temp](const ptr<Message> &message) {
            TEST_ASSERT_TRUE_MESSAGE(temp == *message->payload,
                                     (string("Router retain message payload equality: ") +
                                       router()->pattern()->toString() + " " + temp.toString() +
                                       " != " + message->payload->toString())
                                     .c_str());
          })
        }));
    }
  }
  // if (clearRouter)
  //  Options::singleton()->router<Router>()->clear(false, true);
}
#endif

#ifdef FOS_DEPLOY_PARSER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(
  const List<OBJ> &expected, const List<string> &monoids,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  const bool clearRouter = true) {
  const string &finalString = monoids.back();
  for (size_t i = 0; i < monoids.size() - 1; i++) {
    LOG(DEBUG, FOS_TAB_2 "!yPre-monoid!!: %s\n", monoids.at(i).c_str());
    Fluent(Parser::singleton()->try_parse_obj(monoids.at(i)).value()).iterate();
  }
  LOG(DEBUG, "!gEnd monoid!!: %s\n", finalString.c_str());
  return FOS_CHECK_RESULTS<OBJ>(expected, Fluent(Parser::singleton()->try_parse_obj(finalString).value()),
                                expectedReferences, clearRouter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename OBJ = Obj>
static void FOS_CHECK_RESULTS(
  const List<OBJ> &expected, const string &monoid,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  const bool clearRouter = false) {
  Option<Obj_p> parse = Parser::singleton()->try_parse_obj(monoid);
  if (!parse.has_value())
    throw fError("Unable to parse: %s\n", monoid.c_str());
  return FOS_CHECK_RESULTS<OBJ>(expected, Fluent(parse.value()), expectedReferences, clearRouter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
[[maybe_unused]] static void FOS_SHOULD_RETURN(const List<string> &expected, const string &monoid) {
  Option<Obj_p> parse = Parser::singleton()->try_parse_obj(monoid);
  if (!parse.has_value())
    throw fError("Unable to parse monoid: %s\n", monoid.c_str());
  List<Obj> expectedResults = List<Obj>();
  for (const auto &result: expected) {
    Option<Obj_p> parse2 = Parser::singleton()->try_parse_obj(result);
    if (!parse2.has_value())
      throw fError("Unable to parse expected result: %s\n", result.c_str());
    expectedResults.push_back(*parse2.value());
  }
  return FOS_CHECK_RESULTS<Obj>(expectedResults, Fluent(parse.value()));
}
#endif
#endif
