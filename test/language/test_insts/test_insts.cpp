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

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_SHARED_MEMORY
#include "../test/test_fhatos.hpp"
#include <language/insts.hpp>


namespace fhatos {
  void test_obj(const Obj_p &lhs, const Obj_p &rhs, const Obj_p &expected) {
    LOG(INFO, "Testing %s => %s = %s\n", lhs->toString().c_str(), rhs->toString().c_str(),
        expected->toString().c_str());
    const Obj_p result = rhs->apply(lhs);
    if (*result != *expected) {
      LOG(ERROR, "%s does not equal %s\n", result->toString().c_str(), expected->toString().c_str());
      TEST_FAIL();
    }
  }

  void test_inst(const Obj_p &lhs, const Inst_p &inst, const Obj_p &expected) { test_obj(lhs, inst, expected); }

  ////////////////////////////////////////////////////////////////////////////////////
  void test_plus() {
    test_inst(Obj::to_int(22), //
              Insts::plus(Obj::to_int(10)), //
              Obj::to_int(32));
    test_inst(Obj::to_int(16), //
              Insts::plus(Obj::to_bcode()), //
              Obj::to_int(32));
  }

  void test_mult() {
    // int => mult[int]
    test_inst(Obj::to_int(22), //
              Insts::mult(Obj::to_int(10)), //
              Obj::to_int(220));
    // int => mult[_bcode]
    test_inst(Obj::to_int(16), //
              Insts::mult(Obj::to_bcode()), //
              Obj::to_int(256));
  }

  void test_apply() {
    test_obj(jnt(6), jnt(5), jnt(5));
    test_obj(jnt(3), str("a"), str("a"));
    test_obj(str("a"), str("b"), str("b"));
    test_obj(vri("http://fhatos.org"), lst({jnt(1), jnt(2), jnt(3)}), lst({jnt(1), jnt(2), jnt(3)}));
    /// _bcode => mult[int]
   // test_inst(Obj::to_bcode({Insts::plus(obj(5))}), //
    //          Insts::mult(obj(10)), //
     //         Obj::to_bcode({Insts::plus(obj(5)), Insts::mult(obj(10))}));
    /// _bcode => mult[_bcode]
    //test_inst(Obj::to_bcode({Insts::plus(obj(5))}), //
    //          Insts::mult(Obj::to_bcode({Insts::plus(obj(3))})), //
    //          Obj::to_bcode({Insts::plus(obj(5)), Insts::mult(Obj::to_bcode({Insts::plus(obj(3))}))}));
  }


  void test_group() {
    test_inst(Obj::to_objs({1, 2, 3, 3}), //
              Insts::group(Obj::to_bcode(), Obj::to_bcode(), Obj::to_bcode()), //
              rec({{jnt(1), Obj::to_lst({1})},
                   {jnt(2), Obj::to_lst({2})},
                   {jnt(3), Obj::to_lst({3, 3})}}));
    /* testInst(Insts::group(Obj::to_bcode({}), Obj::to_bcode({}), Obj::to_bcode({Insts::start({}), Insts::count()})),
       // Obj::to_objs({1, 2, 3, 3}), // Objs::to_rec({{1, 1}, {2, 1}, {3, 2}}));*/
  }

  void test_barrier() {
    // <1,2,'a',['x',abc]> =| barrier(count()) => <3>
    test_inst(objs({jnt(1), jnt(2), str("a"), lst({str("x"), vri("abc")})}), //
              Insts::barrier(bcode({Insts::count()})), //
              objs({jnt(4)}));
  }

  void test_within() { test_inst(lst({jnt(1), jnt(2), jnt(3)}), Insts::within(bcode({Insts::sum()})), lst({jnt(6)})); }


  FOS_RUN_TESTS( //
          FOS_RUN_TEST(test_plus); //
          FOS_RUN_TEST(test_mult); //
          FOS_RUN_TEST(test_group); //
          FOS_RUN_TEST(test_apply); //
          FOS_RUN_TEST(test_barrier); //
          FOS_RUN_TEST(test_within); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
