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
      FOS_TEST_FURI_EQUAL(structure->pattern->retract_pattern().extend("xxx"), *p("xxx"));
    }

    [[nodiscard]] fURI_p p(const fURI &furi) const {
      return id_p(prefix_->extend(furi));
    }

    void detach() const {
      structure_->stop();
      Router::singleton()->write(structure_->vid_, Obj::to_noobj());
      Router::singleton()->loop();
      FOS_TEST_ERROR(p("c/23")->toString().append(" -> 23"));
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

    void test_subscribe() const {
      auto counter = new int(0);
      Router::singleton()->subscribe(Subscription::create(
        id_p("tester"),
        p_p(*p("b/#")), [this,counter](const Obj_p &obj, const InstArgs &args) {
          TEST_ASSERT_EQUAL_INT(3, args->rec_value()->size());
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("payload")));
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("target")));
          TEST_ASSERT_EQUAL_INT(1, args->rec_value()->count(vri("retain")));
          TEST_ASSERT_EQUAL_INT(0, args->rec_value()->count(vri("other")));
          FOS_TEST_OBJ_EQUAL(obj, args->arg("payload"));
          TEST_ASSERT_TRUE(args->arg("retain")->bool_value());
          TEST_ASSERT_GREATER_THAN_INT(obj->int_value(), counter);
          FOS_TEST_ASSERT_MATCH_FURI(args->arg("target")->uri_value(), *p("b/#"));
          FOS_TEST_FURI_EQUAL(*p((string("b/b").append(std::to_string(args->arg("payload")->int_value())))),
                              fURI(args->arg("target")->uri_value()));

          *counter = *counter + 1;
          return Obj::to_noobj();
        }));
      for(int i = 0; i < 25; i++) {
        Router::singleton()->write(p(string("b/b").append(to_string(i))), jnt(i));
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      Router::singleton()->unsubscribe(id_p("tester"), p_p(*p("b/#")));
      for(int i = 0; i < 25; i++) {
        Router::singleton()->write(p(string("b/b").append(to_string(i))), jnt(i));
        Router::singleton()->loop();
      }
      TEST_ASSERT_EQUAL_INT(25, *counter);
      delete counter;
      this->detach();
    }
  };
}
