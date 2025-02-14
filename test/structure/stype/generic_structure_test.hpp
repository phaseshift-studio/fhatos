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

#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"
#include "../../../src/structure/structure.hpp"

namespace fhatos {
  class GenericStructureTest {
  protected:
    const Structure_p structure_;
    const ID_p prefix_;

  public:
    explicit GenericStructureTest(const Structure_p &structure): structure_(structure),
                                                                 prefix_(id_p(structure->pattern->retract_pattern())) {
      Router::singleton()->attach(structure);
      TEST_ASSERT_TRUE(structure->available());
      FOS_TEST_FURI_EQUAL(structure->pattern->retract_pattern().extend("xxx"), p("xxx"));
    }

    [[nodiscard]] fURI p(const fURI &furi) const {
      return prefix_->extend(furi);
    }

    void detach() const {
      structure_->stop();
      Router::singleton()->write(structure_->vid, Obj::to_noobj());
      Router::singleton()->loop();
      FOS_TEST_ERROR(p("c/23").toString().append(" -> 23"));
    }

    void test_write() const {
      for(int i = 0; i < 50; i++) {
        structure_->write(p(string("a/a").append(to_string(i))), jnt(i * 10));
      }
      for(int i = 0; i < 50; i++) {
        TEST_ASSERT_EQUAL_INT(i*10, structure_->read(p(string("a/a").append(to_string(i))))->int_value());
      }
      this->detach();
    }

    void test_rec_embedding() const {
      ////////////////////////////
      const Rec_p recA = Obj::to_rec({{"x", jnt(1)}, {"y", jnt(2)}});
      const Rec_p lstA = Obj::to_lst({jnt(1), jnt(2)});
      ROUTER_WRITE(id_p(p("rec/a/b")), recA, true);
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {
            "rec/a/b/c/d/e/f/../..",
            "rec/a/b/c/d/e/../././../f",
            "rec/a/b/c/d/e/./f",
            "rec/a/b/c/d/e",
            "rec/a/b/c/d",
            "rec/a/b"}) {
        const fURI test_furi = p(test_furi_str);
        const std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        FOS_TEST_FURI_EQUAL(p("rec/a/b"), pair->first);
        FOS_TEST_OBJ_EQUAL(recA, pair->second);
      }
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {
            "rec",
            "rec/a",
            "rec/a/b/../..",
            "rec/a/b/..",
            "rec/a/c",
            "rec/a/a/b/c"}) {
        const fURI test_furi = p(test_furi_str);
        std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        TEST_ASSERT_FALSE(pair.has_value());
      }
      ////////////////////////////////////////////////////////////////////////////////////
      /*for(const auto &test_furi_str: {
            "rec/lst/a",
            "rec/lst/a/b",
            "rec/lst/a/b/c"
          }) {
        const fURI_p test_furi = p(test_furi_str);
        Router::singleton()->write(test_furi, lstA);
        FOS_TEST_OBJ_EQUAL(jnt(1), Router::singleton()->read(id_p(test_furi->extend("0"))));
        FOS_TEST_OBJ_EQUAL(jnt(2), Router::singleton()->read(id_p(test_furi->extend("1"))));
        //FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2)}), Router::singleton()->read(test_furi));
        // TODO: should embedding occur with #
        //FOS_TEST_OBJ_EQUAL(rec({{test_furi->name(), lst({jnt(1),jnt(2)})}}),
        //                   Router::singleton()->read(id_p(test_furi->retract().extend("#/"))));
      }*/
      ////////////////////////////////////////////////////////////////////////////////////
      /*for(const auto &test_furi_str: {
            "rec/rec/a",
            "rec/rec/a/b",
            "rec/rec/a/b/c"
          }) {
        const fURI_p test_furi = p(test_furi_str);
        Router::singleton()->write(test_furi, recA);
        FOS_TEST_OBJ_EQUAL(jnt(1), Router::singleton()->read(id_p(test_furi->extend("x"))));
        FOS_TEST_OBJ_EQUAL(jnt(2), Router::singleton()->read(id_p(test_furi->extend("y"))));
        //FOS_TEST_OBJ_EQUAL(rec({{"x",jnt(1)},{"y",jnt(2)}}), Router::singleton()->read(test_furi));
        // TODO: should embedding occur with #
        //FOS_TEST_OBJ_EQUAL(rec({{test_furi->name(), lst({jnt(1),jnt(2)})}}),
        //                   Router::singleton()->read(id_p(test_furi->retract().extend("#/"))));
      }*/
      ////////////////////////////////////////////////////////////////////////////////////
      this->detach();
    }

    void test_lst_embedding() const {
      const Rec_p lstA = Obj::to_lst({jnt(1), jnt(2)});
      const Rec_p recA = Obj::to_rec({{"x", jnt(1)}, {"y", jnt(2)}});
      ROUTER_WRITE(id_p(p("lst/a/b")), lstA, true);
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {
            "lst/a/b/c/d/e/f/../..",
            "lst/a/b/c/d/e/../././../f",
            "lst/a/b/c/d/e/./f",
            "lst/a/b/c/d/e",
            "lst/a/b/c/d",
            "lst/a/b"}) {
        const fURI test_furi = p(test_furi_str);
        const std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        FOS_TEST_FURI_EQUAL(p("/lst/a/b"), pair->first);
        FOS_TEST_OBJ_EQUAL(lstA, pair->second);
      }
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {
            "lst",
            "lst/a",
            "lst/a/b/../..",
            "lst/a/b/..",
            "lst/a/c",
            "lst/a/a/b/c"}) {
        const fURI test_furi = p(test_furi_str);
        std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        TEST_ASSERT_FALSE(pair.has_value());
      }
      ////////////////////////////////////////////////////////////////////////////////////
      /*for(const auto &test_furi_str: {
            "lst/lst/a",
            "lst/lst/a/b",
            "lst/lst/a/b/c"
          }) {
        const fURI_p test_furi = p(test_furi_str);
        Router::singleton()->write(test_furi, lstA);
        //FOS_TEST_OBJ_EQUAL(jnt(1), Router::singleton()->read(id_p(test_furi->extend("0"))));
        //FOS_TEST_OBJ_EQUAL(jnt(2), Router::singleton()->read(id_p(test_furi->extend("1"))));
        //FOS_TEST_OBJ_EQUAL(lst({jnt(1),jnt(2)}), Router::singleton()->read(test_furi));
        // TODO: should embedding occur with #
        //FOS_TEST_OBJ_EQUAL(rec({{test_furi->name(), lst({jnt(1),jnt(2)})}}),
        //                   Router::singleton()->read(id_p(test_furi->retract().extend("#/"))));
      }
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {
            "lst/rec/a",
            "lst/rec/a/b",
            "lst/rec/a/b/c"
          }) {
        const fURI_p test_furi = p(test_furi_str);
        Router::singleton()->write(test_furi, recA);
//        FOS_TEST_OBJ_EQUAL(jnt(1), Router::singleton()->read(id_p(test_furi->extend("x"))));
//        FOS_TEST_OBJ_EQUAL(jnt(2), Router::singleton()->read(id_p(test_furi->extend("y"))));
//        FOS_TEST_OBJ_EQUAL(rec({{"x",jnt(1)},{"y",jnt(2)}}), Router::singleton()->read(test_furi));
        // TODO: should embedding occur with #
        //FOS_TEST_OBJ_EQUAL(rec({{test_furi->name(), lst({jnt(1),jnt(2)})}}),
        //                   Router::singleton()->read(id_p(test_furi->retract().extend("#/"))));
      }*/
      ////////////////////////////////////////////////////////////////////////////////////
      this->detach();
    }

    void test_subscribe() const {
      auto counter = new int(0);
      Router::singleton()->subscribe(Subscription::create(
        id_p("tester"),
        p_p(p("b/#")), [this,counter](const Obj_p &obj, const InstArgs &args) {
          TEST_ASSERT_EQUAL_INT(3, args->rec_value()->size());
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("payload")));
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("target")));
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("retain")));
          TEST_ASSERT_EQUAL_INT(0, args->rec_value()->count(vri("other")));
          FOS_TEST_OBJ_EQUAL(obj, args->arg("payload"));
          TEST_ASSERT_TRUE(args->arg("retain")->bool_value());
          TEST_ASSERT_GREATER_THAN_INT(obj->int_value(), counter);
          FOS_TEST_ASSERT_MATCH_FURI(args->arg("target")->uri_value(), p("b/#"));
          FOS_TEST_FURI_EQUAL(p((string("b/b").append(std::to_string(args->arg("payload")->int_value())))),
                              fURI(args->arg("target")->uri_value()));

          *counter = *counter + 1;
          return Obj::to_noobj();
        }));
      for(int i = 0; i < 25; i++) {
        Router::singleton()->write(id_p(p(string("b/b").append(to_string(i)))), jnt(i));
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      Router::singleton()->unsubscribe("tester", p("b/#"));
      for(int i = 0; i < 25; i++) {
        Router::singleton()->write(id_p(p(string("b/b").append(to_string(i)))), jnt(i));
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      delete counter;
      this->detach();
    }
  };
}
