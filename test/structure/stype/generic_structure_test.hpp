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
#include "../../../src/model/fos/sys/router/router.hpp"
#include "../../../src/model/fos/sys/router/structure/structure.hpp"
#include "../../../src/structure/qtype/q_doc.hpp"
#include "../../test_fhatos.hpp"

#define FOS_MAX_PATH_SEGMENTS 15

namespace fhatos {
  class GenericStructureTest {
  protected:
    const Structure_p structure_;
    const ID_p prefix_;

  public:
    explicit GenericStructureTest(const Structure_p &structure) :
        structure_(structure), prefix_(id_p(structure->pattern->retract_pattern())) {
      Router::singleton()->attach(structure);
      Router::singleton()->loop();
      TEST_ASSERT_TRUE(structure->available());
      // FOS_TEST_FURI_EQUAL(structure->pattern->retract_pattern().extend("xxx"), p("xxx"));
      // Router::singleton()->read(p("#"));
      LOG_WRITE(INFO, structure.get(), L("generic structure test setup for {}", structure->toString()));
    }

    [[nodiscard]] fURI p(const fURI &furi, const char *q = nullptr) const {
      fURI temp = prefix_->extend(furi);
      return q ? temp.query(q) : temp;
    }

    void detach() const {
      // structure_->stop();
      Router::singleton()->loop();
      ROUTER_WRITE(p("#"), Obj::to_noobj(), true); // delete data
      ROUTER_WRITE(*structure_->vid, Obj::to_noobj(), true); // unmount structure
      Router::singleton()->loop();
      FOS_TEST_ERROR(p("c/23").toString().append(" -> 23")); // ensure unmounted
    }

    void test_delete() const {
      for(int i = 0; i < 50; i++) {
        structure_->write(p(string("delete/a").append(to_string(i))), jnt(i * 10));
      }
      TEST_ASSERT_EQUAL_INT(50, structure_->read(p("delete/#"))->objs_value()->size());
      structure_->write(p("delete/#"), Obj::to_noobj());
      for(int i = 0; i < 50; i++) {
        FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), structure_->read(p(string("delete/a").append(to_string(i)))));
      }
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), structure_->read(p("delete/#")));
      this->detach();
    }

    void test_write() const {
      for(int i = 0; i < 50; i++) {
        structure_->write(p(string("a/a").append(to_string(i))), jnt(i * 10));
      }
      for(int i = 0; i < 50; i++) {
        FOS_TEST_OBJ_EQUAL(jnt(i * 10), structure_->read(p(string("a/a").append(to_string(i)))));
      }
      for(int i = 49; i >= 0; i--) {
        FOS_TEST_OBJ_EQUAL(jnt(i * 10), structure_->read(p(string("a/a").append(to_string(i)))));
      }
      for(int i = 0; i < 50; i++) {
        structure_->write(p(string("a/a").append(to_string(i))), Obj::to_noobj());
      }
      for(int i = 0; i < 50; i++) {
        FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), structure_->read(p(string("a/a").append(to_string(i)))));
      }
      for(int i = 49; i >= 0; i--) {
        FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), structure_->read(p(string("a/a").append(to_string(i)))));
      }
      this->detach();
    }

    void test_mono_embedding() const {
      ROUTER_WRITE(p("b"), dool(true), true);
      ROUTER_WRITE(p("i"), jnt(10), true);
      ROUTER_WRITE(p("r"), real(22.5), true);
      ROUTER_WRITE(p("s"), str("fhatty"), true);
      ROUTER_WRITE(p("u"), vri("fhat://pig.com:8080"), true);
      ////////////////////////////////////////////////////
      FOS_TEST_OBJ_EQUAL(dool(true), structure_->read(p("b")));
      FOS_TEST_OBJ_EQUAL(jnt(10), structure_->read(p("i")));
      FOS_TEST_OBJ_EQUAL(real(22.5), structure_->read(p("r")));
      FOS_TEST_OBJ_EQUAL(str("fhatty"), structure_->read(p("s")));
      FOS_TEST_OBJ_EQUAL(vri("fhat://pig.com:8080"), structure_->read(p("u")));
      this->detach();
    }

    void test_rec_embedding() const {
      ////////////////////////////
      const Rec_p recA = Obj::to_rec({{"x", jnt(1)}, {"y", jnt(2)}});
      const Rec_p lstA = Obj::to_lst({jnt(1), jnt(2)});
      ROUTER_WRITE(p("rec/a/b"), recA, true);
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {"rec/a/b/c/d/e/f/../..", "rec/a/b/c/d/e/../././../f", "rec/a/b/c/d/e/./f",
                                      "rec/a/b/c/d/e", "rec/a/b/c/d", "rec/a/b"}) {
        const fURI test_furi = p(test_furi_str);
        const std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        FOS_TEST_FURI_EQUAL(p("rec/a/b"), pair->first);
        FOS_TEST_OBJ_EQUAL(recA, pair->second);
      }
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {"rec", "rec/a", "rec/a/b/../..", "rec/a/b/..", "rec/a/c", "rec/a/a/b/c"}) {
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
      Router::singleton()->write(p("rec/a/b"), Obj::to_noobj());
      this->detach();
    }

    void test_lst_embedding() const {
      const Rec_p lstA = Obj::to_lst({jnt(1), jnt(2)});
      const Rec_p recA = Obj::to_rec({{"x", jnt(1)}, {"y", jnt(2)}});
      ROUTER_WRITE(p("lst/a/b"), lstA, true);
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {"lst/a/b/c/d/e/f/../..", "lst/a/b/c/d/e/../././../f", "lst/a/b/c/d/e/./f",
                                      "lst/a/b/c/d/e", "lst/a/b/c/d", "lst/a/b"}) {
        const fURI test_furi = p(test_furi_str);
        const std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        FOS_TEST_FURI_EQUAL(p("/lst/a/b"), pair->first);
        FOS_TEST_OBJ_EQUAL(lstA, pair->second);
      }
      ////////////////////////////////////////////////////////////////////////////////////
      for(const auto &test_furi_str: {"lst", "lst/a", "lst/a/b/../..", "lst/a/b/..", "lst/a/c", "lst/a/a/b/c"}) {
        const fURI test_furi = p(test_furi_str);
        std::optional<Pair<ID, Obj_p>> pair = structure_->locate_base_poly(test_furi);
        TEST_ASSERT_FALSE(pair.has_value());
      }
      ////////////////////////////////////////////////////////////////////////////////////
      const fURI test_furi = p("ulists");
      Router::singleton()->write(test_furi, lst({vri("a/b/c"), vri("a/b/c/d"), vri("a/b/c/d/e")}));
      FOS_TEST_OBJ_EQUAL(vri("a/b/c"), Router::singleton()->read(test_furi.extend("0")));
      FOS_TEST_OBJ_EQUAL(vri("a/b/c/d"), Router::singleton()->read(test_furi.extend("1")));
      FOS_TEST_OBJ_EQUAL(vri("a/b/c/d/e"), Router::singleton()->read(test_furi.extend("2")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), Router::singleton()->read(test_furi.extend("3")));
      FOS_TEST_OBJ_EQUAL(lst({vri("a/b/c"), vri("a/b/c/d"), vri("a/b/c/d/e")}), Router::singleton()->read(test_furi));
      ////////////////////////////////////////////////////////////////////////////////////
      this->detach();
    }

    void test_q_sub() const {
      // Structure::add_qproc(structure_, QSub::create(structure_->vid->extend("q/sub")));
      const Subscription_p subscription =
          Subscription::create(id_p("/boot/scheduler"), p_p(p("abc")), __().to(p("bcd")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc", "sub")));
      ROUTER_WRITE(p("abc", "sub"), __().to(p("bcd")), true);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(subscription, ROUTER_READ(p("abc", "sub")));
      ////////// transient
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("bcd")));
      ////////// transient
      ROUTER_WRITE(p("abc"), str("fhatty"), false);
      Router::singleton()->loop();
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("fhatty"), ROUTER_READ(p("bcd")));
      ////////// retain
      ROUTER_WRITE(p("abc"), str("fhatty pig"), true);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(str("fhatty pig"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("fhatty pig"), ROUTER_READ(p("bcd")));
      ROUTER_WRITE(p("abc"), str("little fhatty pig"), true);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("bcd")));
      ////////// unsubscribe
      ROUTER_WRITE(p("abc", "sub"), Obj::to_noobj(), true);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc", "sub")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("bcd")));
      ////////// transient (no sub)
      ROUTER_WRITE(p("abc"), str("the fhat"), false);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("bcd")));
      ROUTER_WRITE(p("abc"), str("the little fhat"), false);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("bcd")));
      ////////// retain (no sub)
      ROUTER_WRITE(p("abc"), str("the fhat"), true);
      Router::singleton()->loop();
      FOS_TEST_OBJ_EQUAL(str("the fhat"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("little fhatty pig"), ROUTER_READ(p("bcd")));
      this->detach();
    }

    void test_q_doc() const {
      Structure::add_qproc(structure_, QDoc::create(structure_->vid->extend("q/doc")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc", "doc")));
      ROUTER_WRITE(p("abc"), str("no docs"), true);
      FOS_TEST_OBJ_EQUAL(str("no docs"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc", "doc")));
      ROUTER_WRITE(p("abc", "doc"), str("these are the docs"), true);
      FOS_TEST_OBJ_EQUAL(str("no docs"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("these are the docs"), ROUTER_READ(p("abc", "doc")));
      ROUTER_WRITE(p("abc", "doc"), str("docs updated"), true);
      FOS_TEST_OBJ_EQUAL(str("no docs"), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(str("docs updated"), ROUTER_READ(p("abc", "doc")));
      ROUTER_WRITE(p("abc"), Obj::to_noobj(), true);
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc")));
      FOS_TEST_OBJ_EQUAL(Obj::to_noobj(), ROUTER_READ(p("abc", "doc")));
      this->detach();
    }

    void test_subscribe() const {
      auto counter = new int(0);
      ROUTER_WRITE(p("b/#").query("sub"),
                   Subscription::create(id_p("tester"), p_p(p("b/#")),
                                        [this, counter](const Obj_p &obj, const InstArgs &args) {
                                          TEST_ASSERT_EQUAL_INT(3, args->rec_value()->size());
                                          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("payload")));
                                          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("target")));
                                          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("retain")));
                                          TEST_ASSERT_EQUAL_INT(0, args->rec_value()->count(vri("other")));
                                          FOS_TEST_OBJ_EQUAL(obj, args->arg("payload"));
                                          TEST_ASSERT_TRUE(args->arg("retain")->bool_value());
                                          TEST_ASSERT_GREATER_THAN_INT(obj->int_value(), counter);
                                          FOS_TEST_ASSERT_MATCH_FURI(args->arg("target")->uri_value(), p("b/#"));
                                          FOS_TEST_FURI_EQUAL(p((string("b/b").append(
                                                                  std::to_string(args->arg("payload")->int_value())))),
                                                              fURI(args->arg("target")->uri_value()));

                                          *counter = *counter + 1;
                                          return Obj::to_noobj();
                                        }),
                   true);
      for(int i = 0; i < 25; i++) {
        ROUTER_WRITE(p(string("b/b").append(to_string(i))), jnt(i), true);
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      ROUTER_WRITE(p("b/#").query("sub"), Obj::to_noobj(), true);
      for(int i = 0; i < 25; i++) {
        ROUTER_WRITE(p(string("b/b").append(to_string(i))), jnt(i), true);
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      delete counter;
      this->detach();
    }
  };
} // namespace fhatos
