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

#include <fhatos.hpp>
#include <test_fhatos.hpp>

namespace fhatos {
  void testInst(const Inst_p &inst, const Obj_p &lhs, const Obj_p &expected) {
    LOG(INFO, "Testing %s => %s = %s\n", lhs->toString().c_str(), inst->toString().c_str(),
        expected->toString().c_str());
    const Obj_p result = inst->apply(lhs);
    if (*result != *expected) {
      LOG(ERROR, "%s does not equal %s\n", result->toString().c_str(), expected->toString().c_str());
      TEST_FAIL();
    }
  }

  void test_plus() {
    testInst(Insts::plus(Obj::to_int(10)), //
             Obj::to_int(22), //
             Obj::to_int(32));
    testInst(Insts::plus(Obj::to_bcode({})), //
             Obj::to_int(16), //
             Obj::to_int(32));
  }

  void test_mult() {
    testInst(Insts::mult(Obj::to_int(10)), //
             Obj::to_int(22), //
             Obj::to_int(220));
    testInst(Insts::mult(Obj::to_bcode({})), //
             Obj::to_int(16), //
             Obj::to_int(256));
  }

  void test_group() {
    testInst(Insts::group(Obj::to_bcode({}), Obj::to_bcode({})), //
             Obj::to_objs({1, 2, 3, 3}), //
             Objs::to_rec({{1, {1}}, {2, {2}}, {3, {3, 3}}}));
  }
 
  FOS_RUN_TESTS( //
      for (fhatos::Router *router //
           : List<Router *>{fhatos::LocalRouter::singleton(), //
                            fhatos::MqttRouter::singleton()}) { //
        GLOBAL_OPTIONS->ROUTING = router; //
        LOG(INFO, "!r!_Testing with %s!!\n", router->toString().c_str()); //
        FOS_RUN_TEST(test_plus); //
        FOS_RUN_TEST(test_mult); //
        FOS_RUN_TEST(test_group); //
      })
} // namespace fhatos

SETUP_AND_LOOP();
#endif
