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

   ptr<Heap<>> test_heap = std::make_shared<Heap<>>("/zzz/#");

   // in fhatos namespace
   void setUp(void) {
    static bool first = true;
    if(first) Router::singleton()->attach(test_heap);
    first =false;
   }

    void tearDown(void) {
     ROUTER_WRITE(id_p("/zzz/a/b"),Obj::to_noobj(),true);
     }

  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////

  void test_locate_rec_base() {
    setUp();
    ////////////////////////////
    const Rec_p recA = Obj::to_rec({{"x",jnt(1)},{"y",jnt(2)}});
    ROUTER_WRITE(id_p("/zzz/a/b"),recA,true);
    ////////////////////////////////////////////////////////////////////////////////////
    for(const auto& test_furi_str : {
          "/zzz/a/b/c/d/e/f/../..",
          "/zzz/a/b/c/d/e/../././../f",
          "/zzz/a/b/c/d/e/./f",
          "/zzz/a/b/c/d/e",
          "/zzz/a/b/c/d",
          "/zzz/a/b" }) {
      const ID_p test_furi = id_p(test_furi_str);
      std::optional<Pair<ID_p,Obj_p>> pair = test_heap->locate_base_poly(test_furi);
      FOS_TEST_FURI_EQUAL(ID("/zzz/a/b"),*pair->first);
      FOS_TEST_OBJ_EQUAL(recA, pair->second);
          }
    ////////////////////////////////////////////////////////////////////////////////////
    for(const auto& test_furi_str : {
         "/zzz"
         "/zzz/a",
         "/zzz/a/b/../..",
         "/zzz/a/b/..",
         "/zzz/a/c",
         "/zzz/a/a/b/c" }) {
      const ID_p test_furi = id_p(test_furi_str);
      std::optional<Pair<ID_p,Obj_p>> pair = test_heap->locate_base_poly(test_furi);
      TEST_ASSERT_FALSE(pair.has_value());
      }
    ////////////////////////////////////////////////////////////////////////////////////
  }

  void test_locate_lst_base() {
     setUp();
    const Rec_p lstA = Obj::to_lst({jnt(1),jnt(2)});
    ROUTER_WRITE(id_p("/zzz/a/b"),lstA,true);
    ////////////////////////////////////////////////////////////////////////////////////
    for(const auto& test_furi_str : {
          "/zzz/a/b/c/d/e/f/../..",
          "/zzz/a/b/c/d/e/../././../f",
          "/zzz/a/b/c/d/e/./f",
          "/zzz/a/b/c/d/e",
          "/zzz/a/b/c/d",
          "/zzz/a/b" }) {
      const ID_p test_furi = id_p(test_furi_str);
      std::optional<Pair<ID_p,Obj_p>> pair = test_heap->locate_base_poly(test_furi);
      FOS_TEST_FURI_EQUAL(ID("/zzz/a/b"),*pair->first);
      FOS_TEST_OBJ_EQUAL(lstA, pair->second);
     }
    ////////////////////////////////////////////////////////////////////////////////////
    for(const auto& test_furi_str : {
         "/zzz"
         "/zzz/a",
         "/zzz/a/b/../..",
         "/zzz/a/b/..",
         "/zzz/a/c",
         "/zzz/a/a/b/c" }) {
      const ID_p test_furi = id_p(test_furi_str);
      std::optional<Pair<ID_p,Obj_p>> pair = test_heap->locate_base_poly(test_furi);
      TEST_ASSERT_FALSE(pair.has_value());
     }
    ////////////////////////////////////////////////////////////////////////////////////
   }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_locate_rec_base); //
    FOS_RUN_TEST(test_locate_lst_base); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
