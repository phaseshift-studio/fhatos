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

#ifndef fhatos_base_structure_hpp
#define fhatos_base_structure_hpp

#undef FOS_TEST_ON_BOOT
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY
#include <test_fhatos.hpp>

#define FOS_STOP_ON_BOOT  \
router()->detach(current_structure->pattern());

namespace fhatos {
  inline ptr<Structure> current_structure;

  void test_write() {
    router()->attach(current_structure);
    TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS, router()->write(id_p("/b/c"), jnt(10), id_p("fhatty")));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::NO_TARGETS, router()->write(id_p("a/b/c"), str("hello_pity"), id_p("aus")));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK, router()->write(id_p("/a/b"), str("hello_pity"), id_p("piggy")));
  }

  void test_subscribe() {
    Options::singleton()->log_level(TRACE);
    auto *pings = new atomic_int(0);
   RESPONSE_CODE rc_ = router()->route_subscription(share(Subscription{
      .source = "a/test/case", .pattern = "/a/b/test", .qos = QoS::_1, .onRecv = [pings](const Message_p &message) {
        FOS_TEST_ASSERT_EQUAL_FURI(Pattern("/a/b/test"), message->target);
        if (message->payload->is_bool()) {
          TEST_ASSERT_TRUE(message->payload->bool_value());
          pings->store(pings->load() + 1);
          TEST_ASSERT_EQUAL(1, pings->load());
        }
      }
    }));
    TEST_ASSERT_EQUAL(RESPONSE_CODE::OK,rc_);
    router()->route_message(share(Message{
      .source = "test_case", .target = ID("/a/b/test"), .payload = Obj::to_bool(true), .retain = TRANSIENT_MESSAGE
    }));
  }
} // namespace fhatos

#endif
