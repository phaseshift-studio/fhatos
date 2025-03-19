//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_THREAD_LOCAL
#define ESP_PLATFORM
//#define ARDUINO
#undef ARDUINO
#undef NATIVE
#include "../doctest/doctest.h"
#include "../doctest/runner/runner.h"
#include "../../src/fhatos.hpp"
#include "../../src/furi.hpp"
using namespace fhatos;

/*MAIN(){
  LOG(INFO,"entering main");
  const int argc_ = 3;
  const char *argv_[] = {
    "exe",
    "-d",
    "-s"};
  return doctest::Context(argc_, argv_).run();
}*/


TEST_CASE("furi extend") {
  CHECK(fURI("a/b/c") == fURI("a/b").extend("c"));
}

TEST_CASE("furi resolve") {
  CHECK(fURI("a/c") == fURI("a/b").resolve("./c"));
}
