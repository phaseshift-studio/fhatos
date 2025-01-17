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

//#ifndef FOS_LOGGING
#define FOS_LOGGING INFO
//#endif

#ifndef FOS_TEST_SERIALIZATION
#define FOS_TEST_SERIALIZATION false
#endif

#define RETAIN true

#include "../src/fhatos.hpp"
#include "../build/_deps/unity-src/src/unity.h"
#include "../src/util/options.hpp"
#include "../src/lang/obj.hpp"
#include "../src/furi.hpp"
#include "../src/lang/fluent.hpp"
#include "../src/util/fhat_error.hpp"
#include "../src/structure/stype/heap.hpp"
#include "../src/util/logger.hpp"
#include "../src/model/log.hpp"
#include "../src/util/ansi.hpp"
#include "../src/util/argv_parser.hpp"


#define FOS_DEPLOY_PRINTER

////////////////////////////////////////////// PRINTER ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PRINTER_2                              \
  Options::singleton()->printer<>(Ansi<>::singleton());
#else
#define FOS_DEPLOY_PRINTER_2 ;
#endif
/////////////////////////////////////////// PROCESSOR ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PROCESSOR
#include "../src/lang/processor/processor.hpp"
#define FOS_DEPLOY_PROCESSOR_2 load_processor();
#else
#define FOS_DEPLOY_PROCESSOR_2 ;
#endif
/////////////////////////////////////////// SCHEDULER ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_SCHEDULER
#include "../src/process/ptype/native/scheduler.hpp"
#define FOS_STOP_ON_BOOT  \
  Router::singleton()->stop(); \
  scheduler()->stop();
#define FOS_DEPLOY_SCHEDULER_2  \
  Options::singleton()->scheduler<Scheduler>(Scheduler::singleton("/sys/scheduler/")); \
  Router::singleton()->write(id_p("/sys/router"), Router::singleton());
#else
#define FOS_DEPLOY_SCHEDULER_2 ;
#define FOS_STOP_ON_BOOT ;
#endif
/////////////////////////////////////////// ROUTER //////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_ROUTER
#include "../src/structure/router.hpp"
#include "../src/lang/fluent.hpp"
#include "../src/model/driver/fhatos/core_driver.hpp"
#define FOS_DEPLOY_ROUTER_2 \
  Options::singleton()->router<Router>(Router::singleton());  \
  Router::singleton()->attach(Heap<>::create(Pattern("/sys/#")));        \
  Router::singleton()->attach(Heap<>::create(Pattern("/fos/#")));        \
  void* x = fhatos::FhatOSCoreDriver::import();               \
  Router::singleton()->attach(Heap<>::create(Pattern("/io/log/#")));
#else
#define FOS_DEPLOY_ROUTER_2 ;
#endif
////////////////////////////////////////// PARSER ///////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PARSER
#include "../src/lang/mmadt/parser.hpp"
#include "../src/structure/stype/heap.hpp"
#define FOS_DEPLOY_PARSER_2  \
  Router::singleton()->attach(Heap<>::create(Pattern("/parser/#"))); \
  Router::singleton()->write(id_p("/parser/"), mmadt::Parser::singleton("/parser/"));
#else
#define FOS_DEPLOY_PARSER_2 ;
#endif
//////////////////////////////////////// COMPILER //////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_COMPILER
#include "../src/lang/mmadt/compiler.hpp"
#define FOS_DEPLOY_COMPILER_2 ;
#else
#define FOS_DEPLOY_COMPILER_2 ;
#endif
////////////////////////////////////////// TYPE ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_TYPE
#include "../src/lang/type.hpp"
#include "../src/lang/mmadt/type.hpp"
#include "../src/structure/stype/heap.hpp"
#define FOS_DEPLOY_TYPE_2 \
  Router::singleton()->attach(Heap<>::create(Pattern("/mmadt/#"))); \
  Router::singleton()->write(id_p("/mmadt/"),Typer::singleton("/mmadt/")); \
  mmadt::mmADT::import();
#else
#define FOS_DEPLOY_TYPE_2 ;
#endif
///////////////////////////////////////// HEAP ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_SHARED_MEMORY
#include "../src/structure/stype/heap.hpp"
#define FOS_DEPLOY_SHARED_MEMORY_2 \
  Router::singleton()->attach(Heap<>::create(Pattern((0 ==strcmp("",STR(FOS_DEPLOY_SHARED_MEMORY))) ? \
  "+" : \
  STR(FOS_DEPLOY_SHARED_MEMORY))));
#else
#define FOS_DEPLOY_SHARED_MEMORY_2 ;
#endif
////////////////////////////////////// FILE SYSTEM ////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_FILE_SYSTEM
#include FOS_FILE_SYSTEM(fs.hpp)
#define FOS_DEPLOY_FILE_SYSTEM_2 \
  ptr<FileSystem> fs = FileSystem::create("/fs/#", string(base_directory.c_str()) + "/tmp"); \
  Router::singleton()->attach(fs); \
  fs->setup();
#else
#define FOS_DEPLOY_FILE_SYSTEM_2 ;
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
//////////////////////// NATIVE ////////////////////////
////////////////////////////////////////////////////////
using namespace fhatos;
#define FOS_RUN_TEST(x)                                                                                                \
  {                                                                                                                    \
    try {                                                                                                              \
      RUN_TEST(x);                                                                                                     \
    } catch (const std::exception &e) {                                                                                \
      LOG(ERROR, "failed test due to %s\n", e.what());                                                                 \
      throw;                                                                                                           \
    }                                                                                                                  \
  }

#define FOS_RUN_TESTS(x)                                                                                               \
  void RUN_UNITY_TESTS() {  \
    try {                                                                                                              \
      FOS_DEPLOY_PRINTER_2                                                                                             \
      FOS_DEPLOY_PROCESSOR_2                                                                                           \
      FOS_DEPLOY_SCHEDULER_2                                                                                           \
      FOS_DEPLOY_ROUTER_2                                                                                              \
      FOS_DEPLOY_PARSER_2                                                                                              \
      FOS_DEPLOY_TYPE_2                                                                                                \
      FOS_DEPLOY_SHARED_MEMORY_2                                                                                       \
      FOS_DEPLOY_COMPILER_2																							   \
      FOS_DEPLOY_FILE_SYSTEM_2  																					   \
      UNITY_BEGIN();                                                                                                   \
      /*uint32_t __test_freeSketch;                                                                                    \
      uint32_t __test_freeHeap;  */                                                                                    \
      x;                                                                                                               \
      UNITY_END();                                                                                                     \
    } catch (const std::exception &e) {                                                                                \
      TEST_FAIL_MESSAGE(e.what());                                                                                     \
    }                                                                                                                  \
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//////////////////////// NATIVE ////////////////////////
////////////////////////////////////////////////////////
#ifdef NATIVE
#define SETUP_AND_LOOP_2                                                                                               \
int main(int argc, char ** argv) {                                                                                     \
  Options::singleton()->printer<Ansi<>>(Ansi<>::singleton()); \
  auto *args_parser = new fhatos::ArgvParser();                                                                          \
  args_parser->init(argc, argv);                                                                                       \
  fhatos::Options::singleton()->log_level(                                                                             \
  fhatos::LOG_TYPES.to_enum(args_parser->option_string("--log", STR(FOS_LOGGING))));
/////////////////////////////////////////////////////////
//////////////////////// ESPXX //////////////////////////
/////////////////////////////////////////////////////////
#else
#define SETUP_AND_LOOP_2                                                                                               \
  void setup() {                                                                                                       \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                                                                 \
    delay(2000);
#endif

#define SETUP_AND_LOOP()                                                                                               \
  using namespace fhatos; \
  SETUP_AND_LOOP_2                                                                                                     \
  RUN_UNITY_TESTS();                                                                                                   \
  FOS_STOP_ON_BOOT;                                                                                                    \
};

void loop() {
}

void setUp() {
}

void tearDown() {
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// TEST UTILITY FUNCTIONS /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace fhatos;

static auto serialization_check = [](const Obj_p& obj) -> Obj_p {
  if(FOS_TEST_SERIALIZATION) {
    BObj_p bobj = obj->serialize();
    return Obj::deserialize(bobj);
  }
  return obj;
};

#define PROCESS_ALL(bcode_string) \
  BCODE_PROCESSOR(OBJ_PARSER((bcode_string)))


#define PROCESS(bcode_string) BCODE_PROCESSOR(serialization_check(OBJ_PARSER((bcode_string))))->objs_value(0)

#define FOS_TEST_MESSAGE(format, ...) \
  if (fhatos::LOG_TYPE::FOS_LOGGING < fhatos::LOG_TYPE::ERROR) {                                                                         \
    Ansi<>::singleton()->printf((format), ##__VA_ARGS__);                                                                          \
    Ansi<>::singleton()->println();                                                                                                \
    Ansi<>::singleton()->printf("  !rline %s:%i!!\t\n", __FILE__, __LINE__);                                                         \
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

#define FOS_TEST_COMPILER_TRUE(x,y,compiler_f)                                                                         \
  FOS_TEST_MESSAGE("!b%s!! =!rcompiler true!!= !b%s!!", (x)->toString().c_str(), (y)->toString().c_str());             \
  TEST_ASSERT_TRUE(compiler_f(x,y));

#define FOS_TEST_COMPILER_FALSE(x,y,compiler_f)                                                                        \
  FOS_TEST_MESSAGE("!b%s!! =!r%s false!!= !b%s!!",                                                                     \
     (x)->toString().c_str(), "compiler", (y)->toString().c_str());                                                    \
  TEST_ASSERT_FALSE(compiler_f(x,y));

#define FOS_TEST_EXCEPTION_CXX(x)                                                                                      \
  try {                                                                                                                \
    (x);                                                                                                               \
    TEST_ASSERT(false);                                                                                                \
  } catch (const fError &e) {                                                                                          \
    FOS_TEST_MESSAGE("!gexpected error occurred!!: %s", e.what());                                                     \
    TEST_ASSERT(true);                                                                                                 \
  }

#ifdef FOS_DEPLOY_PARSER
#define FOS_TEST_ASSERT_EXCEPTION(fn)                                                                                  \
  try {                                                                                                                \
    (fn)();                                                                                                            \
    TEST_FAIL_MESSAGE("!rno exception occurred!!: " STR(__FILE__) ":" STR(__LINE__));                                  \
  } catch (const fError &e) {                                                                                          \
    FOS_TEST_MESSAGE("!gexpected exception occurred!!: %s", e.what());                                                 \
    TEST_ASSERT(true);                                                                                                 \
  }
#endif

#define FOS_TEST_OBJ_EQUAL(objA, objB)                                                                                 \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!yTesting equality!! : %s %s %s", (objA)->toString().c_str(), test ? "==" : "!=", (objB)->toString().c_str());  \
    if (!test) TEST_FAIL_MESSAGE("failure: " STR(__FILE__) ":" STR(__LINE__));                                         \
  }
#define FOS_TEST_OBJ_NTEQL(objA, objB) FOS_TEST_OBJ_NOT_EQUAL((objA),(objB))
#define FOS_TEST_OBJ_NOT_EQUAL(objA, objB)                                                                             \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!yTesting not equal!!: %s %s %s", (objA)->toString().c_str(),                                    \
                     test ? "==" : "!=", (objB)->toString().c_str());                                                  \
    if (test)                                                                                                          \
     TEST_FAIL_MESSAGE("failure: " STR(__FILE__) ":" STR(__LINE__));                                                   \
  }

//#ifdef FOS_DEPLOY_PARSER
static ptr<List<Obj_p>> FOS_TEST_RESULT(const BCode_p &bcode, const bool print_result = true) {
  FOS_TEST_MESSAGE("!yTesting!!: %s", bcode->toString().c_str());
  if(!bcode->is_bcode())
    return std::make_shared<List<Obj_p>>(List<Obj_p>{bcode});
  List_p<Obj_p> result = BCODE_PROCESSOR(bcode)->objs_value();
  if(print_result) {
    int index = 0;
    for(const auto &obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       OTypes.to_chars(obj->o_type()).c_str());
    }
  }
  return result;
}

//#endif

#define FOS_TEST_OBJ_GT(obj_a, obj_b)                                                                                  \
  FOS_TEST_MESSAGE("!yTesting greater than!! : %s %s %s", obj_a->toString().c_str(),                                   \
                   (*obj_a > *obj_b) ? ">" : "!=", obj_b->toString().c_str());                                         \
  if (!(*obj_a > *obj_b))                                                                                              \
    TEST_FAIL();

#define FOS_TEST_OBJ_LT(obj_a, obj_b)                                                                                  \
  FOS_TEST_MESSAGE("!yTesting less than!! : %s %s %s", obj_a->toString().c_str(),                                      \
  (*obj_a < *obj_b) ? "<" : "!=", obj_b->toString().c_str());                                                          \
  if (!(*obj_a < *obj_b))                                                                                              \
    TEST_FAIL();

#define FOS_PRINT_OBJ(obj) \
  FOS_TEST_MESSAGE("!yTesting!!: %s [otype:!y%s!!][itype:!y%s!!]", obj->toString().c_str(), \
                   OTypes.to_chars(obj->o_type()).c_str(), ITypeDescriptions.to_chars(obj->itype()).c_str());

#ifdef FOS_DEPLOY_PARSER
[[maybe_unused]] static void FOS_TEST_ERROR(const string &monoid) {
  try {
    PROCESS(monoid)->objs_value();
    TEST_ASSERT_TRUE_MESSAGE(false, ("no exception thrown in " + monoid).c_str());
  } catch(const fError &error) {
    LOG(INFO, "expected !rexception thrown!!: %s\n", error.what());
    TEST_ASSERT_TRUE(true);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_ROUTER
/*[[maybe_unused]] static void FOS_CHECK_RESULTS(
  const List<Obj> &expected, const BCode_p &bcode,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  [[maybe_unused]] const bool clearRouter = true) {
   ptr<List<ptr<Obj>>> result = FOS_TEST_RESULT(bcode, true);
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected.size(), result->size(), "Expected result size");
  for(const Obj &obj: expected) {
    auto x = std::find_if(result->begin(), result->end(), [obj](const Obj_p &element) {
      if(obj.is_real()) {
        return obj.real_value() + 0.01f > element->real_value() && obj.real_value() - 0.01f < element->real_value();
      } else
        return obj == *element;
    });
    if(result->end() == x) {
      TEST_FAIL_MESSAGE(("Unable to find " + obj.toString()).c_str());
    }
  }
  if(!expectedReferences.empty()) {
    /* TEST_ASSERT_EQUAL_INT_MESSAGE(
         expectedReferences.size(), Router::singleton()->retainSize(),
         (string("Router retain message count: ") + Router::singleton()->pattern()->toString()).c_str());*/
/*    for(const auto &[key, value]: expectedReferences) {
      const Obj temp = value;
      ROUTER_SUBSCRIBE(
        Subscription::create(ID("fhatty"),
                             key.uri_value(),
                             InstBuilder::build()->inst_f([temp](const ptr<Rec> &message, const InstArgs &args) {
                               TEST_ASSERT_TRUE_MESSAGE(temp == *args->arg(0),
                                                        (string("Router retain message payload equality: ") +
                                                          Router::singleton()->vid()->toString() + " " + temp.toString() +
                                                          " != " + message->rec_get("payload")->toString())
                                                        .c_str());
                               return noobj();
                             })->create()));
    }
  }
  // if (clearRouter)
  //  Options::singleton()->router<Router>()->clear(false, true);
}*/
#endif

#ifdef FOS_DEPLOY_PARSER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*[[maybe_unused]] static void FOS_CHECK_RESULTS(
  const List<Obj> &expected, const List<string> &monoids,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  const bool clearRouter = true) {
  const string &finalString = monoids.back();
  for (size_t i = 0; i < monoids.size() - 1; i++) {
    LOG(DEBUG, FOS_TAB_2 "!yPre-monoid!!: %s\n", monoids.at(i).c_str());
    Fluent(Parser::singleton()->try_parse_obj(monoids.at(i)).value()).iterate();
  }
  LOG(DEBUG, "!gEnd monoid!!: %s\n", finalString.c_str());
  return FOS_CHECK_RESULTS(expected, Parser::singleton()->try_parse_obj(finalString).value(),
                           expectedReferences, clearRouter);
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*[[maybe_unused]] static void FOS_CHECK_RESULTS(
  const List<Obj> &expected, const string &monoid,
  const Map<Uri, Obj, Obj::obj_comp> &expectedReferences = {},
  const bool clearRouter = false) {
  Option<Obj_p> parse = {OBJ_PARSER(monoid)};
  if (!parse.has_value())
    throw fError("Unable to parse: %s\n", monoid.c_str());
  return FOS_CHECK_RESULTS(expected, parse.value(), expectedReferences, clearRouter);
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*[[maybe_unused]] static void FOS_SHOULD_RETURN(const List<string> &expected, const string &monoid) {
  const Option<Obj_p> parse = Parser::singleton()->try_parse_obj(monoid);
  if (!parse.has_value())
    throw fError("Unable to parse monoid: %s\n", monoid.c_str());
  auto expectedResults = List<Obj>();
  for (const auto &result: expected) {
    Option<Obj_p> parse2 = Parser::singleton()->try_parse_obj(result);
    if (!parse2.has_value())
      throw fError("Unable to parse expected result: %s\n", result.c_str());
    expectedResults.push_back(*parse2.value());
  }
  return FOS_CHECK_RESULTS(expectedResults, parse.value());
}*/
#endif
#endif
