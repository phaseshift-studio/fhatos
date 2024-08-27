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

#undef FOS_TEST_ON_BOOT

#include <language/parser.hpp>
#include <test_fhatos.hpp>
#include <util/obj_helper.hpp>
#include <structure/router.hpp>

namespace fhatos {

  void test_subscribe() {}

  void test_publish() {
    /* TEST_ASSERT_EQUAL(
         RESPONSE_CODE::NO_TARGETS,
         LocalRouter::singleton()->publish(Message{
             .source = ID("a"), .target = ID("b"), .payload = share<Str>(Str("test")), .retain = TRANSIENT_MESSAGE}));*/
  }

  void test_bobj_wrap() {
    Parser::singleton();
    const List<ID_p> ids = {id_p("fos://127.0.0.1/here"), id_p("/stuff/stuff"),
                            id_p("//fhat/os?a=b&c=d"), id_p("a"), id_p("fos:abc")};
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
      for (const auto &i: ids) {
        const BObj_p bobj = Message::wrapSource(i, o);
        const auto [id, obj] = Message::unwrapSource(bobj);
        LOG(INFO, "%s\n", obj->toString().c_str());
        FOS_TEST_ASSERT_EQUAL_FURI(*i, *id);
        FOS_TEST_OBJ_EQUAL(o, obj);
      }
    }
  }

  FOS_RUN_TESTS( //
          Options::singleton()->log_level(LOG_TYPE::TRACE); //
          FOS_RUN_TEST(test_publish); //
          FOS_RUN_TEST(test_bobj_wrap); //
  );

} // namespace fhatos

SETUP_AND_LOOP();

#endif
