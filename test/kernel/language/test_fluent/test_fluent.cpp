#ifndef fhatos_kernel__test_fluent_hpp
#define fhatos_kernel__test_fluent_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/language/fluent.hpp>
#include <kernel/language/instructions.hpp>
#include <kernel/language/obj.hpp>
#include <unity.h>

namespace fhatos::kernel {

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void test_fluent() {
  FOS_TEST_MESSAGE("%s",
                   __<Int>({10}).plus(Int(20)).plus(Int(5)).toString().c_str());
  FOS_TEST_MESSAGE("%s", __<Int>(10)
                             .plus(__<Int>().plus(6).plus(12).plus(
                                 __<Int>().plus(13).plus(6)))
                             .toString()
                             .c_str());
  FOS_TEST_MESSAGE("%s", Bytecode<Int, Int>(Int(10)).toString().c_str());

  FOS_TEST_MESSAGE("%s", Monad<Int>(new Int(32))
                             .split(new PlusInst<Int>(10))
                             ->get()
                             ->toString()
                             .c_str());

  __<Int>({32, 45, 1, 2}).plus(10).plus(15).forEach([](const Int *e) {
    FOS_TEST_MESSAGE("=>%s", e->toString().c_str());
  });

    FOS_TEST_MESSAGE("=========================\n");

 /*const Fluent<Int,Int> f=  __<Int>({32, 45, 1, 2}).plus(10).plus(15);
  for(int i =0;i<4;i++) {
      FOS_TEST_MESSAGE("=>%s",f.next()->toString().c_str());
  }*/
}

FOS_RUN_TESTS(                 //
    FOS_RUN_TEST(test_fluent); //

);

} // namespace fhatos::kernel

SETUP_AND_LOOP();

#endif