
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

#include <test_fhatos.hpp>
#include <util/uri.hpp>
namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_abc() {
    for (const string &uri:
         List<string>({"//a", "http://127.0.0.1", "http://127.0.0.1/a/b/c", "http://127.0.0.1:80/a/b/c",
                       "furi://a.b.c/a", "furi://a.b.c:8080/a", "furi://user:pass@127.0.0.1/a/b/c", "//127.0.0.1:80",
                       "/int/nat/even", "/int/", "/a/b/c", "furi:a/b/c"})) {
      UriX u = UriX(uri);
      LOG(INFO, "!b%s!!\t!g%s!!\n", uri.c_str(), u.toString().c_str());
      TEST_ASSERT_EQUAL_STRING(uri.c_str(), u.toString().c_str());
    }
  }

  // furi://user:pass@127.0.0.1:88/a/b/c?x=1&y=2#fhatty
  void test_components() {
    const List<Pair<string, List<int>>> uris = List<Pair<string, List<int>>>({
        /*{"furi:", {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}*/
        {"furi://127.0.0.1", {1, 0, 0, 1, 0, 0, 0, 0, 0, 0}},
        {"//127.0.0.1", {0, 0, 0, 1, 0, 0, 0, 0, 0, 0}},
        {"//127.0.0.1:88", {0, 0, 0, 1, 88, 0, 0, 0, 0, 0}},
        {"//user@127.0.0.1", {0, 1, 0, 1, 0, 0, 0, 0, 0, 0}},
        {"//user@127.0.0.1:88", {0, 1, 0, 1, 88, 0, 0, 0, 0, 0}},
        {"furi://user@127.0.0.1", {1, 1, 0, 1, 0, 0, 0, 0, 0, 0}},
        {"furi://user:pass@127.0.0.1", {1, 1, 1, 1, 0, 0, 0, 0, 0, 0}},
        {"furi://127.0.0.1:88", {1, 0, 0, 1, 88, 0, 0, 0, 0, 0}},
        {"furi://127.0.0.1:88/a", {1, 0, 0, 1, 88, 1, 0, 0, 0, 0}},
        {"furi://127.0.0.1:88//bb", {1, 0, 0, 1, 88, 0, 1, 0, 0, 0}},
        {"furi://127.0.0.1:88//bb/c_c_c", {1, 0, 0, 1, 88, 0, 1, 1, 0, 0}},
        {"furi://127.0.0.1:88///c_c_c", {1, 0, 0, 1, 88, 0, 0, 1, 0, 0}},
        {"furi://127.0.0.1:88/a/bb", {1, 0, 0, 1, 88, 1, 1, 0, 0, 0}},
        {"furi://127.0.0.1:88/a/bb/c_c_c", {1, 0, 0, 1, 88, 1, 1, 1, 0, 0}},
        //{"furi://127.0.0.1:88/a///c_c_c", {1, 0, 0, 1, 88, 1, 0, 1, 0, 0}},
        {"furi://127.0.0.1:88/a/bb/c_c_c?x=1&y=2", {1, 0, 0, 1, 88, 1, 1, 1, 1, 0}},
        {"furi://127.0.0.1:88/a/bb/c_c_c?x=1&y=2#fhatty", {1, 0, 0, 1, 88, 1, 1, 1, 1, 1}},
        {"furi://127.0.0.1:88/a/bb#fhatty", {1, 0, 0, 1, 88, 1, 1, 0, 0, 1}},
        {"furi:a", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a/", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a/bb", {1, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
        {"a", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"/a", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"/a/", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"/a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        {"a#fhatty", {0, 0, 0, 0, 0, 1, 0, 0, 0, 1}},
        {"a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        {"/a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        //{"/a/bb/#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        {"a?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        {"/a?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        //{"/a/?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        {"/a/bb?x=1&y=2", {0, 0, 0, 0, 0, 1, 1, 0, 1, 0}},
        {"/a/bb?x=1&y=2#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 1, 1}},
        //{"/a//c_c_c?x=1&y=2#fhatty", {0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
        //{"furi:/a//c_c_c?x=1&y=2", {1, 0, 0, 0, 0, 1, 0, 1, 1, 0}}
    });

    for (Pair<string, List<int>> pair: uris) {
      TEST_ASSERT_EQUAL_INT(10, pair.second.size());
      UriX uri = UriX(pair.first);
      TEST_ASSERT_EQUAL_STRING(pair.first.c_str(), uri.toString().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(uri, UriX(uri.toString()));
      /////////
      if (pair.second.at(0))
        TEST_ASSERT_EQUAL_STRING("furi", uri._scheme);
      else
        TEST_ASSERT_NULL(uri._scheme);
      /////////
      if (pair.second.at(1))
        TEST_ASSERT_EQUAL_STRING("user", uri._user);
      else
        TEST_ASSERT_NULL(uri._user);
      /////////
      if (pair.second.at(2))
        TEST_ASSERT_EQUAL_STRING("pass", uri._password);
      else
        TEST_ASSERT_NULL(uri._password);
      /////////
      if (pair.second.at(3))
        TEST_ASSERT_EQUAL_STRING("127.0.0.1", uri._host);
      else
        TEST_ASSERT_NULL(uri._host);
      /////////
      if (pair.second.at(4))
        TEST_ASSERT_EQUAL_INT(88, uri._port);
      else
        TEST_ASSERT_EQUAL_INT(0, uri._port);
      /////////
      bool path = true;
      if (pair.second.at(5))
        TEST_ASSERT_EQUAL_STRING("a", uri._path[0]);
      else if (pair.second.at(6) == 0 && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(0, uri._pathLength);
      }
      /////////
      if (pair.second.at(6))
        TEST_ASSERT_EQUAL_STRING("bb", uri._path[1]);
      else if (path && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(1, uri._pathLength);
      }
      /////////
      if (pair.second.at(7))
        TEST_ASSERT_EQUAL_STRING("c_c_c", uri._path[2]);
      else if (path) {
        TEST_ASSERT_EQUAL_INT(2, uri._pathLength);
      }
      /////////
      if (pair.second.at(8))
        TEST_ASSERT_EQUAL_STRING("x=1&y=2", uri._query);
      else {
        TEST_ASSERT_NULL(uri._query);
      }
      /////////
      if (pair.second.at(9))
        TEST_ASSERT_EQUAL_STRING("fhatty", uri._fragment);
      else {
        TEST_ASSERT_NULL(uri._fragment);
      }
    }
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_components); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
