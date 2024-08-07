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

#ifndef fhatos_test_insts_cpp
#define fhatos_test_insts_cpp

#undef FOS_TEST_ON_BOOT

#include <fhatos.hpp>
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <test_fhatos.hpp>

namespace fhatos {
  void testObj(const Obj_p &lhs, const Obj_p &rhs, const Obj_p &expected) {
    LOG(INFO, "Testing %s => %s = %s\n", lhs->toString().c_str(), rhs->toString().c_str(),
        expected->toString().c_str());
    const Obj_p result = rhs->apply(lhs);
    if (*result != *expected) {
      LOG(ERROR, "%s does not equal %s\n", result->toString().c_str(), expected->toString().c_str());
      TEST_FAIL();
    }
  }
  void testInst(const Obj_p &lhs, const Inst_p &inst, const Obj_p &expected) { testObj(lhs, inst, expected); }
  ////////////////////////////////////////////////////////////////////////////////////
  void test_plus() {
    testInst(Obj::to_int(22), //
             Insts::plus(Obj::to_int(10)), //
             Obj::to_int(32));
    testInst(Obj::to_int(16), //
             Insts::plus(Obj::to_bcode()), //
             Obj::to_int(32));
  }

  void test_mult() {
    // int => mult[int]
    testInst(Obj::to_int(22), //
             Insts::mult(Obj::to_int(10)), //
             Obj::to_int(220));
    // int => mult[bcode]
    testInst(Obj::to_int(16), //
             Insts::mult(Obj::to_bcode()), //
             Obj::to_int(256));
  }

  void test_apply() {
    testObj(obj(6), obj(5), obj(5));
    testObj(obj(3), obj("a"), obj("a"));
    testObj(obj("a"), obj("b"), obj("b"));
    testObj(obj(u("http://fhatos.org")), obj({1, 2, 3}), obj({1, 2, 3}));
    /// bcode => mult[int]
    testInst(Obj::to_bcode({Insts::plus(obj(5))}), //
             Insts::mult(obj(10)), //
             Obj::to_bcode({Insts::plus(obj(5)), Insts::mult(obj(10))}));
    /// bcode => mult[bcode]
    testInst(Obj::to_bcode({Insts::plus(obj(5))}), //
             Insts::mult(Obj::to_bcode({Insts::plus(obj(3))})), //
             Obj::to_bcode({Insts::plus(obj(5)), Insts::mult(Obj::to_bcode({Insts::plus(obj(3))}))}));
  }


  void test_group() {
    testInst(Obj::to_objs({1, 2, 3, 3}), //
             Insts::group(Obj::to_bcode(), Obj::to_bcode(), Obj::to_bcode()), //
             Objs::to_rec({{1, *Obj::to_lst({1})}, {2, *Obj::to_lst({2})}, {3, *Obj::to_lst({3, 3})}}));
    /* testInst(Insts::group(Obj::to_bcode({}), Obj::to_bcode({}), Obj::to_bcode({Insts::start({}), Insts::count()})),
       // Obj::to_objs({1, 2, 3, 3}), // Objs::to_rec({{1, 1}, {2, 1}, {3, 2}}));*/
  }

  void test_barrier() {
    // <1,2,'a',['x',abc]> =| barrier(count()) => <3>
    testInst(objs({obj(1), obj(2), obj("a"), lst({obj("x"), uri("abc")})}), //
             Insts::barrier(bcode({Insts::count()})), //
             objs({obj(4)}));
  }


  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_plus); //
      FOS_RUN_TEST(test_mult); //
      FOS_RUN_TEST(test_group); //
      FOS_RUN_TEST(test_apply); //
      FOS_RUN_TEST(test_barrier); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
