#ifndef fhatos_test_fluent_hpp
#define fhatos_test_fluent_hpp

#include <test_fhatos.hpp>
#include <language/fluent.hpp>
#include <language/instructions.hpp>
#include <language/obj.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_fluent() {
    FOS_TEST_MESSAGE("%s",
                     __<Int>(10).plus(20).plus(__<Int>(26).plus(5)).plus(5).toString().c_str());
    FOS_TEST_MESSAGE("%s", __<Int>(10)
                     .plus(__<Int>().plus(Int(6)).plus(Int(12)).plus(
                       __<Int>().plus(Int(13)).plus(Int(6)))).plus(23)
                     .toString()
                     .c_str());

    FOS_TEST_MESSAGE("%s", (new Monad<Int>(new Int(32)))->split(new PlusInst<Int>(new Int(10)))
                     ->get()
                     ->toString()
                     .c_str());

  __<Int>({32,45}).plus(10).plus(15).forEach([](const Int *e){
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });

    FOS_TEST_MESSAGE("=========================\n");

    const Fluent<Int, Int> f = __<Int>({232, 3145, 13}).plus(10).plus(15);
    f.forEach([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_fluent); //

  );
} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
