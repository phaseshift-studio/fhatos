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

#ifndef fhatos_test_router_hpp
#define fhatos_test_router_hpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#include <test_fhatos.hpp>
#include <util/obj_helper.hpp>


namespace fhatos {

  void test_subscribe() {}

  void test_publish() {
    /* TEST_ASSERT_EQUAL(
         RESPONSE_CODE::NO_TARGETS,
         LocalRouter::singleton()->publish(Message{
             .source = ID("a"), .target = ID("b"), .payload = share<Str>(Str("test")), .retain = TRANSIENT_MESSAGE}));*/
  }

  void test_bobj() {
    Parser::singleton();
    const List<Obj_p> objs = {Obj::to_int(1),
                              Obj::to_int(-453),
                              Obj::to_real(12.035f),
                              Obj::to_str("fhatos"),
                              Obj::to_uri("aaaa"),
                              Obj::to_lst({1, 7, "abc", u("hello/fhat/aus")}),
                              Obj::to_rec({{u("a"), 2},
                                           {u("b"), 3}}),
                              Obj::to_noobj()};
    for (const auto &o: objs) {
        const BObj_p bobj = o->serialize();
        const Obj_p obj = Obj::deserialize<Obj>(bobj);
        LOG(INFO, "%s\n", obj->toString().c_str());
        FOS_TEST_OBJ_EQUAL(o, obj);
    }
  }

  FOS_RUN_TESTS( //
          FOS_RUN_TEST(test_publish); //
          FOS_RUN_TEST(test_bobj); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
