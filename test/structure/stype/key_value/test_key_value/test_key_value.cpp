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

#ifndef fhatos_test_key_value_hpp
#define fhatos_test_key_value_hpp

#include <../test/structure/test_base_structure.hpp>
#include <structure/stype/key_value.hpp>

namespace fhatos {
  FOS_RUN_TESTS( //
    current_structure = KeyValue::create("/a/+"); //
    //router()->attach(current_structure); //
    current_structure->setup(); //
    FOS_RUN_TEST(test_write); //
    FOS_RUN_TEST(test_subscribe); //
    current_structure->stop(); //
    //router()->detach(current_structure->pattern()); //
  );
} // namespace fhatos
SETUP_AND_LOOP();
#endif
