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

#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"
#include "../generic_structure_test.hpp"
#include STR(../../../../src/structure/stype/mqtt/HARDWARE/mqtt.hpp)

namespace fhatos {
  using namespace mmadt;
  //Mqtt(const Pattern &pattern, const ID_p &value_id = nullptr, const Rec_p &config = Obj::to_rec()) :
  const Structure_p test_mqtt = std::make_shared<Mqtt>("/xyz/#", id_p("/sys/test"),
  Obj::to_rec({{"broker",vri("mqtt://localhost:1883")},
               {"client",vri("test_client")}}));
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////

  void test_heap_generic_write() { GenericStructureTest(test_mqtt).test_write(); }

  void test_heap_generic_subscribe() { GenericStructureTest(test_mqtt).test_subscribe(); }

  void test_heap_generic_lst_embedding() { GenericStructureTest(test_mqtt).test_lst_embedding(); }

  void test_heap_generic_rec_embedding() { GenericStructureTest(test_mqtt).test_rec_embedding(); }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_heap_generic_write); //
      FOS_RUN_TEST(test_heap_generic_subscribe); //
      //FOS_RUN_TEST(test_heap_generic_lst_embedding); //
      FOS_RUN_TEST(test_heap_generic_rec_embedding); //
      );

} // namespace fhatos

SETUP_AND_LOOP();
