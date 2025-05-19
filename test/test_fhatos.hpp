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

// #ifndef FOS_LOGGING
#define FOS_LOGGING INFO
// #endif

#ifndef FOS_TEST_SERIALIZATION
#define FOS_TEST_SERIALIZATION false
#endif

#define RETAIN true
#define FOS_MAX_PATH_SEGMENTS 15

#include "../src/kernel.hpp"
#include "../build/_deps/unity-src/src/unity.h"
#include "../src/boot_config_loader.hpp"
#include "../src/fhatos.hpp"
#include "../src/furi.hpp"
#include "../src/lang/obj.hpp"
#include "../src/model/fos/fos_obj.hpp"
#include "../src/model/fos/sys/router/router.hpp"
#include "../src/model/fos/sys/router/structure/heap.hpp"
#include "../src/model/fos/sys/scheduler/scheduler.hpp"
#include "../src/model/fos/ui/terminal.hpp"
#include "../src/model/fos/util/log.hpp"
#include "../src/util/ansi.hpp"
#include "../src/util/argv_parser.hpp"
#include "../src/util/fhat_error.hpp"
#include "../src/util/logger.hpp"
#include "../src/util/options.hpp"
#include "../src/util/print_helper.hpp"

#define FOS_DEPLOY_ROUTER_2 ;
#define FOS_STOP_ROUTER ;
#define FOS_DEPLOY_SCHEDULER_2 ;
#define FOS_STOP_SCHEDULER ;
#define FOS_DEPLOY_PARSER_2 ;
#define FOS_STOP_PARSER ;
#define FOS_DEPLOY_COMPILER_2 ;

////////////////////////////////////////////// PRINTER ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PRINTER_2 ;
#endif
/////////////////////////////////////////// PROCESSOR ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PROCESSOR
#include "../src/lang/processor/processor.hpp"
#define FOS_DEPLOY_PROCESSOR_2 ;
#else
#define FOS_DEPLOY_PROCESSOR_2 ;
#endif
/////////////////////////////////////////// SCHEDULER ///////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_SCHEDULER
#include "../src/model/fos/sys/scheduler/scheduler.hpp"
#define FOS_STOP_SCHEDULER Scheduler::singleton()->stop();
#define FOS_DEPLOY_SCHEDULER_2                                                                                         \
  Scheduler::singleton("/sys/scheduler");                                                                              \
  Kernel::using_scheduler("scheduler");
#endif
/////////////////////////////////////////// ROUTER //////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_ROUTER
#include "../src/kernel.hpp"
#include "../src/model/fos/sys/router/router.hpp"
#include "../src/model/fos/sys/router/structure/heap.hpp"
// FOS_BOOT_CONFIG_HEADER_URI;
#define FOS_STOP_ROUTER Router::singleton()->stop();
#define FOS_DEPLOY_ROUTER_2                                                                                            \
  Router::singleton("/sys/router");                                                                                    \
  load_processor();                                                                                                    \
  Kernel::mount(Heap<>::create("/sys/#"))                                                                              \
      ->mount(Heap<>::create("/mnt/+"))                                                                                \
      ->mount(Heap<>::create("/boot/#", id_p("/mnt/boot")))                                                            \
      ->mount(Heap<>::create("/io/#", id_p("/mnt/io")))                                                                \
      ->mount(Heap<>::create("/mmadt/#", id_p("/mnt/mmadt")));                                                         \
  Kernel::using_boot_config("/../test/data/boot/test_boot_config.obj");                                                \
  LOG_WRITE(INFO, Router::singleton().get(), L("{}\n", Kernel::boot()->toString()));                                   \
  Kernel::using_router("router");
#endif
////////////////////////////////////////// PARSER ///////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_PARSER
#include "../src/lang/mmadt/parser.hpp"
#include "../src/model/fos/sys/router/structure/heap.hpp"
#define FOS_DEPLOY_PARSER_2                                                                                            \
  Kernel::install(                                                                                                     \
      mmadt::Parser::singleton("/io/parser", Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID "/parser")));
#endif
//////////////////////////////////////// COMPILER //////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_COMPILER
#include "../src/lang/mmadt/compiler.hpp"
#define FOS_DEPLOY_COMPILER_2 ;
#endif
////////////////////////////////////////// TYPE ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_MMADT_TYPE
#include "../src/lang/mmadt/mmadt_obj.hpp"
#include "../src/model/fos/sys/typer/typer.hpp"
#include "../src/model/fos/sys/router/structure/heap.hpp"
#define FOS_DEPLOY_MMADT_TYPE_2                                                                                        \
  Processor::import();                                                                                                 \
  load_processor();                                                                                                    \
  mmadt::mmADT::import({"/mmadt/#"});
#else
#define FOS_DEPLOY_MMADT_TYPE_2 ;
#endif
////////////////////////////////////////// EXT ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_MMADT_EXT_TYPE
#include "../src/lang/mmadt/mmadt_obj.hpp"
#include "../src/model/fos/sys/typer/typer.hpp"
#define FOS_DEPLOY_MMADT_EXT_TYPE_2 mmadt::mmADT::import({"/mmadt/ext/+"});
#else
#define FOS_DEPLOY_MMADT_EXT_TYPE_2 ;
#endif
////////////////////////////////////////// FOS ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_FOS_TYPE
#include "../src/model/fos/sys/typer/typer.hpp"
#include "../src/model/fos/fos_obj.hpp"
#include "../src/model/fos/sys/router/structure/heap.hpp"
#define FOS_DEPLOY_FOS_TYPE_2                                                                                          \
  Kernel::mount(Heap<>::create("/fos/#", id_p("/mnt/fos")));                                                           \
  Typer::singleton("/sys/typer");                                                                                      \
  LOG_WRITE(INFO, Typer::singleton().get(), L("{}\n", Kernel::boot()->toString()));                                    \
  Kernel::using_typer("typer");
#else
#define FOS_DEPLOY_FOS_TYPE_2 ;
#endif
///////////////////////////////////////// HEAP ////////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_SHARED_MEMORY
#include "../src/model/fos/sys/router/structure/heap.hpp"
#define FOS_DEPLOY_SHARED_MEMORY_2                                                                                     \
  Kernel::mount(Heap<>::create(string(STR(FOS_DEPLOY_SHARED_MEMORY)).empty() ? "+/#" : STR(FOS_DEPLOY_SHARED_MEMORY),  \
                               id_p("/mnt/var")));
#else
#define FOS_DEPLOY_SHARED_MEMORY_2 ;
#endif
////////////////////////////////////// FILE SYSTEM ////////////////////////////////////////////////////////////
#ifdef FOS_DEPLOY_FILE_SYSTEM
#include "../src/model/fos/io/fs/fs.hpp"
#define FOS_DEPLOY_FILE_SYSTEM_2 Kernel::mount(FS::create("/fs/#", id_p("/mnt/fs")));
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
    } catch(const std::exception &e) {                                                                                 \
      LOG_WRITE(ERROR, Obj::to_noobj().get(), L("failed test due to {}\n", e.what()));                                 \
      TEST_FAIL_MESSAGE("failed test");                                                                                \
    }                                                                                                                  \
  }

#define FOS_RUN_TESTS(x)                                                                                               \
  void RUN_UNITY_TESTS() {                                                                                             \
    try {                                                                                                              \
      FOS_DEPLOY_ROUTER_2                                                                                              \
      FOS_DEPLOY_SCHEDULER_2                                                                                           \
      FOS_DEPLOY_PARSER_2                                                                                              \
      FOS_DEPLOY_FOS_TYPE_2                                                                                            \
      FOS_DEPLOY_MMADT_TYPE_2                                                                                          \
      FOS_DEPLOY_MMADT_EXT_TYPE_2                                                                                      \
      FOS_DEPLOY_SHARED_MEMORY_2                                                                                       \
      FOS_DEPLOY_FILE_SYSTEM_2                                                                                         \
      BOOTING = false;                                                                                                 \
      UNITY_BEGIN();                                                                                                   \
      x;                                                                                                               \
      UNITY_END();                                                                                                     \
    } catch(const std::exception &e) {                                                                                 \
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
  int main(int argc, char **argv) {                                                                                    \
    auto *args_parser = new fhatos::ArgvParser();                                                                      \
    args_parser->init(argc, argv);                                                                                     \
    fhatos::LOG_LEVEL = fhatos::LOG_TYPES.to_enum(args_parser->option_string("--log", STR(FOS_LOGGING)));
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
  using namespace fhatos;                                                                                              \
  SETUP_AND_LOOP_2                                                                                                     \
  RUN_UNITY_TESTS();                                                                                                   \
  }                                                                                                                    \
  ;

void loop() {}

void setUp() {}

void tearDown() {}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// TEST UTILITY FUNCTIONS /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace fhatos;

static auto serialization_check = [](const Obj_p &obj) -> Obj_p {
  if constexpr(FOS_TEST_SERIALIZATION) {
    BObj_p bobj = obj->serialize();
    return Obj::deserialize(bobj);
  }
  return obj;
};

#define PROCESS_ALL(bcode_string) BCODE_PROCESSOR(OBJ_PARSER((bcode_string)))


#define PROCESS(bcode_string) BCODE_PROCESSOR(serialization_check(OBJ_PARSER((bcode_string))))->objs_value(0)

#define FOS_TEST_MESSAGE(format, ...)                                                                                  \
  if(fhatos::LOG_TYPE::FOS_LOGGING < fhatos::LOG_TYPE::ERROR) {                                                        \
    Ansi<>::singleton()->printf((format), ##__VA_ARGS__);                                                              \
    Ansi<>::singleton()->println();                                                                                    \
    Ansi<>::singleton()->printf("  !rline %s:%i!!\t\n", __FILE__, __LINE__);                                           \
  }

#define FOS_TEST_FURI_EQUAL(x, y)                                                                                      \
  FOS_TEST_MESSAGE("!ytesting equality!!: <!b%s!!> =!r?!!= <!b%s!!> (%i !rchar_length!! %i) (%i !rpath_length!! %i)",  \
                   (x).toString().c_str(), (y).toString().c_str(), (x).toString().length(), (y).toString().length(),   \
                   (x).path_length(), (y).path_length());                                                              \
  TEST_ASSERT_TRUE_MESSAGE((x).equals(y), "Not equals()");                                                             \
  TEST_ASSERT_TRUE_MESSAGE((x) == (y), "Not ==");                                                                      \
  TEST_ASSERT_EQUAL_STRING((x).toString().c_str(), (y).toString().c_str());

#define FOS_TEST_ASSERT_NOT_EQUAL_FURI(x, y)                                                                           \
  FOS_TEST_MESSAGE(                                                                                                    \
      "!ytesting non equal!!: <!b%s!!> =!r/?!!= <!b%s!!> (%i !rchar_length!! %i) (%i !rpath_length!! %i)",             \
      (x).toString().c_str(), (y).toString().c_str(), (x).toString().length(), (y).toString().length(),                \
      (x).path_length(), (y).path_length());                                                                           \
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

#define FOS_TEST_COMPILER_TRUE(x, y, compiler_f)                                                                       \
  FOS_TEST_MESSAGE("!b%s!! =!rcompiler true!!= !b%s!!", (x)->toString().c_str(), (y).toString().c_str());              \
  TEST_ASSERT_TRUE(compiler_f(x, y));

#define FOS_TEST_COMPILER_FALSE(x, y, compiler_f)                                                                      \
  FOS_TEST_MESSAGE("!b%s!! =!r%s false!!= !b%s!!", (x)->toString().c_str(), "compiler", (y).toString().c_str());       \
  TEST_ASSERT_FALSE(compiler_f(x, y));

#define FOS_TEST_EXCEPTION_CXX(x)                                                                                      \
  try {                                                                                                                \
    (x);                                                                                                               \
    TEST_FAIL_MESSAGE("!rno exception occurred!!: " STR(__FILE__) ":" STR(__LINE__));                                  \
  } catch(const fError &e) {                                                                                           \
    FOS_TEST_MESSAGE("!gexpected error occurred!!: %s", e.what());                                                     \
  }

#ifdef FOS_DEPLOY_PARSER
#define FOS_TEST_ASSERT_EXCEPTION(fn)                                                                                  \
  try {                                                                                                                \
    (fn)();                                                                                                            \
    TEST_FAIL_MESSAGE("!rno exception occurred!!: " STR(__FILE__) ":" STR(__LINE__));                                  \
  } catch(const fError &e) {                                                                                           \
    FOS_TEST_MESSAGE("!gexpected exception occurred!!: %s", e.what());                                                 \
    TEST_ASSERT(true);                                                                                                 \
  }
#endif

#define FOS_TEST_REC_KEYS(recA, list_of_keys)                                                                          \
  {                                                                                                                    \
    for(const Obj_p &e: list_of_keys) {                                                                                \
      bool found = false;                                                                                              \
      for(const auto &[k, v]: *recA->rec_value()) {                                                                    \
        if(e->equals(*k))                                                                                              \
          found = true;                                                                                                \
      }                                                                                                                \
      if(!found)                                                                                                       \
        TEST_FAIL_MESSAGE((string("key:") + e->toString() + " not found in " + recA->toString()).c_str());             \
    }                                                                                                                  \
  }

#define FOS_TEST_OBJ_EQUAL(objA, objB)                                                                                 \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!ytesting equality!! : %s %s %s", (objA)->toString().c_str(),                                    \
                     test ? "==" : "!=", (objB)->toString().c_str());                                                  \
    if(!test)                                                                                                          \
      TEST_FAIL_MESSAGE("failure: " STR(__FILE__) ":" STR(__LINE__));                                                  \
  }
#define FOS_TEST_OBJ_NTEQL(objA, objB) FOS_TEST_OBJ_NOT_EQUAL((objA), (objB))
#define FOS_TEST_OBJ_NOT_EQUAL(objA, objB)                                                                             \
  {                                                                                                                    \
    const bool test = *(objA) == *(objB);                                                                              \
    FOS_TEST_MESSAGE("!ytesting not equal!!: %s %s %s", (objA)->toString().c_str(),                                    \
                     test ? "==" : "!=", (objB)->toString().c_str());                                                  \
    if(test)                                                                                                           \
      TEST_FAIL_MESSAGE("failure: " STR(__FILE__) ":" STR(__LINE__));                                                  \
  }

// #ifdef FOS_DEPLOY_PARSER
static ptr<List<Obj_p>> FOS_TEST_RESULT(const BCode_p &bcode, const bool print_result = true) {
  FOS_TEST_MESSAGE("!ytesting!!: %s", bcode->toString().c_str());
  if(!bcode->is_bcode())
    return std::make_shared<List<Obj_p>>(List<Obj_p>{bcode});
  List_p<Obj_p> result = BCODE_PROCESSOR(bcode)->objs_value();
  if(print_result) {
    int index = 0;
    for(const auto &obj: *result) {
      FOS_TEST_MESSAGE(FOS_TAB_2 "!g=%i!!=>%s [!y%s!!]", index++, obj->toString().c_str(),
                       OTypes.to_chars(obj->otype).c_str());
    }
  }
  return result;
}

// #endif

#define FOS_TEST_OBJ_GT(obj_a, obj_b)                                                                                  \
  FOS_TEST_MESSAGE("!ytesting greater than!! : %s %s %s", obj_a->toString().c_str(),                                   \
                   (*obj_a > *obj_b) ? ">" : "!=", obj_b->toString().c_str());                                         \
  if(!(*obj_a > *obj_b))                                                                                               \
    TEST_FAIL();

#define FOS_TEST_OBJ_LT(obj_a, obj_b)                                                                                  \
  FOS_TEST_MESSAGE("!ytesting less than!! : %s %s %s", obj_a->toString().c_str(),                                      \
                   (*obj_a < *obj_b) ? "<" : "!=", obj_b->toString().c_str());                                         \
  if(!(*obj_a < *obj_b))                                                                                               \
    TEST_FAIL();

#define FOS_PRINT_OBJ(obj)                                                                                             \
  FOS_TEST_MESSAGE("!ytesting!!: %s [otype:!y%s!!][itype:!y%s!!]", obj->toString().c_str(),                            \
                   OTypes.to_chars(obj->otype).c_str(), ITypeDescriptions.to_chars(obj->itype()).c_str());

#ifdef FOS_DEPLOY_PARSER
[[maybe_unused]] static void FOS_TEST_ERROR(const string &monoid) {
  try {
    const Obj_p result = PROCESS(monoid);
    FOS_TEST_MESSAGE("!rno exception thrown!! in %s: %s", monoid.c_str(), result->toString().c_str());
    TEST_ASSERT_TRUE(false);
  } catch(const fError &error) {
    FOS_TEST_MESSAGE("!gexpected !rexception thrown!!: %s", error.what());
    // LOG_WRITE(INFO, Scheduler::singleton().get(), L("expected !rexception thrown!!: {}\n", error.what()));
    TEST_ASSERT_TRUE(true);
  }
}
#endif
#endif
