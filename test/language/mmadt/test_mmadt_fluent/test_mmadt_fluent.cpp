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
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_SHARED_MEMORY
#define FOS_DEPLOY_PROCESSOR
#include "../../../../src/lang/mmadt/mmadt.hpp"
#include "../../../test_fhatos.hpp"

namespace fhatos {
  using namespace mmadt;
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_identity() { FOS_TEST_OBJ_EQUAL(jnt(2), __(jnt(2)).compute().next()); }

  void test_to_from() {
    FOS_TEST_OBJ_EQUAL(jnt(2), __(jnt(1)).to(vri("a")).plus(__().from(vri("a"))).compute().next());
    // FOS_CHECK_RESULTS({23}, __(10).to(*vri("b")).plus(3).plus(_.from(*vri("b")))._bcode, {{*vri("b"), 10}});
    // FOS_CHECK_RESULTS({"fhatos"}, __("fhat").to(*vri("c")).plus("os")._bcode, {{*vri("c"), "fhat"}});
  }

  void test_relational_predicates() {
    //// INT
    FOS_TEST_OBJ_EQUAL(jnt(1), __(jnt(1)).is(__().eq(jnt(1))).compute().next());
    // FOS_TEST_OBJ_EQUAL({}, __(1).is(_.neq(1))._bcode);
    FOS_TEST_OBJ_EQUAL(jnt(12), __(objs({jnt(1), jnt(2), jnt(3)})).plus(jnt(10)).is(__().eq(jnt(12))).compute().next());
    // FOS_TEST_OBJ_EQUAL({11, 13}, __({1, 2, 3}).plus(10).is(_.neq(12))._bcode);
    // FOS_TEST_OBJ_EQUAL(jnt(13), __({1, 2, 3}).plus(10).is(_.gt(12))._bcode);
    // FOS_TEST_OBJ_EQUAL({12, 13}, __({1, 2, 3}).plus(10).is(_.gte(12))._bcode);
    // FOS_TEST_OBJ_EQUAL(jnt(11), __({1, 2, 3}).plus(10).is(_.lt(12))._bcode);
    // FOS_TEST_OBJ_EQUAL({11, 12}, __({1, 2, 3}).plus(10).is(_.lte(12))._bcode);
  }
  /*
  //// REAL
  FOS_CHECK_RESULTS({1.0f}, __(1.0f).is(_.eq(1.0f))._bcode);
  FOS_CHECK_RESULTS({}, __(1.0f).is(_.neq(1.0f))._bcode);
  FOS_CHECK_RESULTS({12.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.eq(12.0f))._bcode);
  FOS_CHECK_RESULTS({11.0f, 13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.neq(12.0f))._bcode);
  FOS_CHECK_RESULTS({13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gt(12.0f))._bcode);
  FOS_CHECK_RESULTS({12.0f, 13.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.gte(12.0f))._bcode);
  FOS_CHECK_RESULTS({11.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lt(12.0f))._bcode);
  FOS_CHECK_RESULTS({11.0f, 12.0f}, __({1.0f, 2.0f, 3.0f}).plus(10.0f).is(_.lte(12.0f))._bcode);
  //// STR
  FOS_CHECK_RESULTS({"1"}, __("1").is(_.eq("1"))._bcode);
  FOS_CHECK_RESULTS({}, __("1").is(_.neq("1"))._bcode);
  FOS_CHECK_RESULTS({"20"}, __({"1", "2", "3"}).plus("0").is(_.eq("20"))._bcode);
  FOS_CHECK_RESULTS({"10", "30"}, __({"1", "2", "3"}).plus("0").is(_.neq("20"))._bcode);
  FOS_CHECK_RESULTS({"30"}, __({"1", "2", "3"}).plus("0").is(_.gt("20"))._bcode);
  FOS_CHECK_RESULTS({"20", "30"}, __({"1", "2", "3"}).plus("0").is(_.gte("20"))._bcode);
  FOS_CHECK_RESULTS({"10"}, __({"1", "2", "3"}).plus("0").is(_.lt("20"))._bcode);
  FOS_CHECK_RESULTS({"10", "20"}, __({"1", "2", "3"}).plus("0").is(_.lte("20"))._bcode);
}

void test_plus() {
  FOS_CHECK_RESULTS({true}, __(true).plus(false)._bcode);
  FOS_CHECK_RESULTS({true}, __(true).plus(true)._bcode);
  FOS_CHECK_RESULTS({false, true, false}, __(List<Obj>{false, true, false}).plus(false)._bcode);
  //
  FOS_CHECK_RESULTS({3}, __(1).plus(2)._bcode);
  FOS_CHECK_RESULTS({54, 50, 46}, __({1, 2, 3}).plus(10).plus(_).plus(_.plus(2))._bcode);
  //
  FOS_CHECK_RESULTS({46.5f}, __(1.121f).plus(10.002f).plus(_).plus(_.plus(2.0f))._bcode);
  FOS_CHECK_RESULTS({54.4f, 50.4f, 46.4f}, __({1.05f, 2.05f, 3.05f}).plus(10.05f).plus(_).plus(_.plus(2.0f))._bcode);
  //
  FOS_CHECK_RESULTS(
      {*Obj::to_rec({{"a", 1},
                     {"b", 2},
                     {"c", 3},
                     {"d", 4}})},
      __(*Obj::to_rec({{"a", 1}})).plus(*Obj::to_rec({{"b", 2}})).plus(*Obj::to_rec({{"c", 3},
        {"d", 4}}))._bcode);
}

void test_mult() {
  // URI
  FOS_CHECK_RESULTS({*vri("http://fhatos.org/b")},
__(*vri("http://fhatos.org")).mult(*vri("/a")).mult(*vri("b"))._bcode); FOS_CHECK_RESULTS({*vri("http://fhatos.org/b")},
__(*vri("http://fhatos.org")).mult(*vri("/a")).mult(*vri("/b"))._bcode);
  FOS_CHECK_RESULTS({*vri("http://fhatos.org/a/b")},
__(*vri("http://fhatos.org")).mult(*vri("/a/")).mult(*vri("b"))._bcode);
  // FOS_CHECK_RESULTS<Uri>({u("http://fhatos.org/a/b/")}, __(u("http://fhatos.org")).mult(u("/a/")).mult(u("b/")));
  FOS_CHECK_RESULTS({*vri("http://fhatos.org/b/")},
__(*vri("http://fhatos.org")).mult(*vri("/a/")).mult(*vri("../b/"))._bcode);
  FOS_CHECK_RESULTS({*vri("http://fhatos.org/b")},
__(*vri("http://fhatos.org")).mult(*vri("/a/")).mult(*vri("../b"))._bcode);
  // FOS_CHECK_RESULTS<Rec>({Rec{{21, 10}, {48, 36}}}, __(Rec{{3, 2}, {6, 4}}).mult(Rec{{7, 5}, {8, 9}}));
}

void test_count() {
  FOS_CHECK_RESULTS({3}, __({5, 4, 7}).plus(2).count()._bcode);
  FOS_CHECK_RESULTS({1}, __({5, 4, 7}).plus(2).count().count()._bcode);
}*/

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_identity); //
      FOS_RUN_TEST(test_to_from); //
      // FOS_RUN_TEST(test_plus); //
      //   FOS_RUN_TEST(test_mult); //
      //   FOS_RUN_TEST(test_count); //
      FOS_RUN_TEST(test_relational_predicates); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
