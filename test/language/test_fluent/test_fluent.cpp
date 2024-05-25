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
                     __<Int>(19).plus(__<Int>(26).plus(5)).plus(5).toString().c_str());
    FOS_TEST_MESSAGE("%s", __<Int>(10)
                     .plus(__<Int>().plus(6).mult(23).plus(
                       __<Int>().plus(13).plus(6))).plus(23)
                     .toString()
                     .c_str());

    FOS_TEST_MESSAGE("%s", (new Monad<Int>(new Int(32)))->split<Int>((Inst*)(new PlusInst<Int>(new Int(10))))
                     ->get()
                     ->toString()
                     .c_str());

    __<Int>({32, 45}).plus(10).plus(15).forEach([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });

    FOS_TEST_MESSAGE("=========================\n");

    const Fluent<Int, Int> f = __<Int>(30).plus(10).plus(15).mult(__<Int>().plus(5)); //.mult(__.plus(2).mult(10));
    f.forEach([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });
  }

  void test_rec_branch() {
    Fluent<Int, Int> f =
        __<Int>(1).plus(2).branch({
            {_<Int>.is(_<Int>.eq(3)), _<Int>.plus(2)},
            {2, 4},
            {_<Int>.mult(2), 7}
        }).is(_<Bool>.eq(5)).mult(_<Int>.plus(95));
    FOS_TEST_MESSAGE("%s",
                     f.toString().c_str());
    f.forEach([](const Int *e) {
      FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
    });
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_fluent); //
      FOS_RUN_TEST(test_rec_branch); //

      );
} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif
