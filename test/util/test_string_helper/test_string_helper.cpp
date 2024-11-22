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

#ifndef fhatos_test_mutex_hpp
#define fhatos_test_mutex_hpp

#include <test_fhatos.hpp>
#include <util/string_helper.hpp>

const int MAX_INTEGER = 5;

namespace fhatos {

  void test_hex_manipulations() {
    for (int i = 0; i < MAX_INTEGER; i++) {
      string x_from_i = StringHelper::int_to_hex(i);
      List<fbyte> b_from_x = StringHelper::hex_to_bytes(x_from_i);
      string x_from_b = StringHelper::bytes_to_hex(b_from_x);
      //printf("%i-[%X]-> %s = %s\n", i, i, x_from_i.c_str(), x_from_b.c_str());
      TEST_ASSERT_EQUAL_STRING(x_from_b.c_str(), x_from_i.c_str());
    }
  }
  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_hex_manipulations); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif