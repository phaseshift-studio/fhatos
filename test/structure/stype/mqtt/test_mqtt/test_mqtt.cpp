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

#ifndef fhatos_test_mqtt_cpp
#define fhatos_test_mqtt_cpp

#include "../../../test_base_structure.hpp"
#include FOS_MQTT(mqtt.hpp)

namespace fhatos {
  FOS_RUN_TESTS( //
    begin_test_structure(Mqtt::create("//test/#",Mqtt::Settings())); //
    FOS_RUN_TEST(test_subscribe); //
    //FOS_RUN_TEST(test_data_types); //
    FOS_RUN_TEST(test_write); //
    FOS_RUN_TEST(test_read); //
    FOS_RUN_TEST(test_patterned_reads); //
    FOS_RUN_TEST(test_ided_reads); //
    FOS_RUN_TEST(test_embedding); //
    FOS_RUN_TEST(test_from_at); //
    end_test_structure();
  );
} // namespace fhatos
SETUP_AND_LOOP();
#endif
