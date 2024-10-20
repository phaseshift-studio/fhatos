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
#define FOS_DEPLOY_SHARED_MEMORY /+/
#define FOS_DEPLOY_EXT
#include <test_fhatos.hpp>
#include <util/obj_helper.hpp>


namespace fhatos {

  void test_subscribe() {
  }

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
                              Obj::to_lst({jnt(1), jnt(7), str("abc"), vri("hello/fhat/aus")}),
                              Obj::to_rec({{vri("a"), jnt(2)},
                                           {vri("b"), jnt(3)}}),
                              Obj::to_noobj()};
    for (const auto &o: objs) {
      const BObj_p bobj = o->serialize();
      const Obj_p obj = Obj::deserialize(bobj);
      LOG(INFO, "%s\n", obj->toString().c_str());
      FOS_TEST_OBJ_EQUAL(o, obj);
    }
  }

  void test_subscriptions() {
    auto counter = new uint8_t(0);
    router()->route_subscription(subscription_p(ID("a_source"), Pattern("/abc/"), Insts::to_bcode(
                                                    [counter](const Message_p &message) {
                                                      LOG(INFO, "Message received: %s\n", message->toString().c_str());
                                                      (*counter)++;
                                                      TEST_ASSERT_EQUAL_INT(134, message->payload->int_value());
                                                    })));

    router()->write(id_p("/abc/"), jnt(134));
    router()->loop();
    while (*counter < 1) {
      // waiting
    }
    TEST_ASSERT_EQUAL_INT(1, *counter);
    delete counter;
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_subscriptions); //
      FOS_RUN_TEST(test_publish); //
      FOS_RUN_TEST(test_bobj); //
      );

} // namespace fhatos

SETUP_AND_LOOP();

#endif