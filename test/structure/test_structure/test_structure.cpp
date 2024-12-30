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

#ifndef fhatos_test_structure_cpp
#define fhatos_test_structure_cpp
#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /abc/#
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"

namespace fhatos {

  void test_locate_poly_base() {
   const ptr<Heap<>> test_heap = std::make_shared<Heap<>>(Obj::to_rec({{"pattern",vri("/zzz/#")}}));
    router()->attach(test_heap);
    const Rec_p recA = Obj::to_rec({{"x",jnt(1)},{"y",jnt(2)}});
    ROUTER_WRITE(id_p("/zzz/a/b"),recA,true);
    ////////////////////////////
     std::optional<Pair<ID_p,Obj_p>> pair = test_heap->locate_base_poly(furi_p("/zzz/a/b/c/d"));
    TEST_ASSERT_TRUE(pair.has_value());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/zzz/a/b"),*pair->first);
    FOS_TEST_OBJ_EQUAL(recA, pair->second);
    ////////////////////////////
    pair = test_heap->locate_base_poly(furi_p("/zzz/a/b/c/d/e"));
    TEST_ASSERT_TRUE(pair.has_value());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/zzz/a/b"),*pair->first);
    FOS_TEST_OBJ_EQUAL(recA, pair->second);
    ////////////////////////////
    pair = test_heap->locate_base_poly(furi_p("/zzz/a/b"));
    TEST_ASSERT_TRUE(pair.has_value());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/zzz/a/b"),*pair->first);
    FOS_TEST_OBJ_EQUAL(recA, pair->second);
    ////////////////////////////
    pair = test_heap->locate_base_poly(furi_p("/zzz/z/y"));
    TEST_ASSERT_FALSE(pair.has_value());
    }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_locate_poly_base); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
