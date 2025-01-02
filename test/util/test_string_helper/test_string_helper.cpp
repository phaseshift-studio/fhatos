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

#ifndef fhatos_test_string_helper_cpp
#define fhatos_test_string_helper_cpp

#include "../../test_fhatos.hpp"
#include "../../../src/util/string_helper.hpp"

const int MAX_INTEGER = 5;

namespace fhatos {
  using namespace std;
  void test_hex_manipulations() {
    for (int i = 0; i < 1000; i++) {
      string x_from_i = StringHelper::int_to_hex(i);
      List<fbyte> b_from_x = StringHelper::hex_to_bytes(x_from_i);
      string x_from_b = StringHelper::bytes_to_hex(b_from_x);
      //printf("%i-[%X]-> %s = %s\n", i, i, x_from_i.c_str(), x_from_b.c_str());
      TEST_ASSERT_EQUAL_STRING(x_from_b.c_str(), x_from_i.c_str());
    }
  }

  void test_tokenize() {
    vector<string> tokens = StringHelper::tokenize(',',"3,4");
    TEST_ASSERT_EQUAL_INT(2,tokens.size());
    TEST_ASSERT_EQUAL_STRING("3",tokens.at(0).c_str());
    TEST_ASSERT_EQUAL_STRING("4",tokens.at(1).c_str());
    ////
    vector<int> tokens_i = StringHelper::tokenize<int>(',',"3,4",
                                                       [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(2,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(3,tokens_i.at(0));
    TEST_ASSERT_EQUAL_INT(4,tokens_i.at(1));
    ///////////////////////////////////////////////////
    tokens = StringHelper::tokenize(',',",5");
    TEST_ASSERT_EQUAL_INT(2,tokens.size());
    TEST_ASSERT_EQUAL_STRING("",tokens.at(0).c_str());
    TEST_ASSERT_EQUAL_STRING("5",tokens.at(1).c_str());
    ////
    tokens_i = StringHelper::tokenize<int>(',',",5",
                                           [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(2,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(-1,tokens_i.at(0));
    TEST_ASSERT_EQUAL_INT(5,tokens_i.at(1));
    ///////////////////////////////////////////////////
    tokens = StringHelper::tokenize(',',"6,");
    TEST_ASSERT_EQUAL_INT(2,tokens.size());
    TEST_ASSERT_EQUAL_STRING("6",tokens.at(0).c_str());
    TEST_ASSERT_EQUAL_STRING("",tokens.at(1).c_str());
    ////
    tokens_i = StringHelper::tokenize<int>(',',"6,",
                                         [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(2,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(6,tokens_i.at(0));
    TEST_ASSERT_EQUAL_INT(-1,tokens_i.at(1));
    ///////////////////////////////////////////////////
    tokens = StringHelper::tokenize(',',",");
    TEST_ASSERT_EQUAL_INT(2,tokens.size());
    TEST_ASSERT_EQUAL_STRING("",tokens.at(0).c_str());
    TEST_ASSERT_EQUAL_STRING("",tokens.at(1).c_str());
    ////
    tokens_i = StringHelper::tokenize<int>(',',",",
                                     [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(2,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(-1,tokens_i.at(0));
    TEST_ASSERT_EQUAL_INT(-1,tokens_i.at(1));
    ///////////////////////////////////////////////////
    tokens = StringHelper::tokenize(',',"7");
    TEST_ASSERT_EQUAL_INT(1,tokens.size());
    TEST_ASSERT_EQUAL_STRING("7",tokens.at(0).c_str());
    ////
    tokens_i = StringHelper::tokenize<int>(',',"7",
                                     [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(1,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(7,tokens_i.at(0));
    ///////////////////////////////////////////////////
    tokens = StringHelper::tokenize(',',"");
    TEST_ASSERT_EQUAL_INT(1,tokens.size());
    TEST_ASSERT_EQUAL_STRING("",tokens.at(0).c_str());
    ////
    tokens_i = StringHelper::tokenize<int>(',',"",
                                     [](const string& s) {return StringHelper::is_integer(s) ? stoi(s) : -1; });
    TEST_ASSERT_EQUAL_INT(1,tokens_i.size());
    TEST_ASSERT_EQUAL_INT(-1,tokens_i.at(0));
    ///////////////////////////////////////////////////
  }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_hex_manipulations); //
    FOS_RUN_TEST(test_tokenize); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif