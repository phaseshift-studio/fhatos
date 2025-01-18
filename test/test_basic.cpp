#include "../build/_deps/catch2-src/src/catch2/catch_test_macros.hpp"
#include "../build/_deps/catch2-src/src/catch2/benchmark/catch_benchmark.hpp"
#include "../src/lang/obj.hpp"

using namespace fhatos;
TEST_CASE("basic int construction", "[basic_int]" ) {
  INFO("testing int");
  REQUIRE( jnt(1)->int_value() == 1 );
  REQUIRE( *jnt(1) == *jnt(1) );
  REQUIRE( *jnt(1)->tid_ == *INT_FURI );
  /////////////////////////////////////
  /*BENCHMARK("int"){
    return jnt(6);
  };
  BENCHMARK("<tid>int"){
    return jnt(6,INT_FURI);
  };
  BENCHMARK("<tid>int@<vid>"){
    return jnt(6,INT_FURI,"abc");
  };*/
};