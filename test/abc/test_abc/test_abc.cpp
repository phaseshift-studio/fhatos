
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

#include <test_fhatos_fast.hpp>
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
  void test_uri_components() {
    const List<Pair<string, List<int>>> uris = List<Pair<string, List<int>>>({
        {"furi:", {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
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
      TEST_ASSERT_EQUAL_STRING(pair.second.at(0) ? "furi" : "", uri.scheme());
      TEST_ASSERT_EQUAL_STRING(pair.second.at(1) ? "user" : "", uri.user());
      TEST_ASSERT_EQUAL_STRING(pair.second.at(2) ? "pass" : "", uri.password());
      TEST_ASSERT_EQUAL_STRING(pair.second.at(3) ? "127.0.0.1" : "", uri.host());
      TEST_ASSERT_EQUAL_INT(pair.second.at(4) ? 88 : 0, uri.port());
      bool path = true;
      if (pair.second.at(5))
        TEST_ASSERT_EQUAL_STRING("a", uri.path(0));
      else if (pair.second.at(6) == 0 && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(0, uri.path_length());
      }
      /////////
      if (pair.second.at(6))
        TEST_ASSERT_EQUAL_STRING("bb", uri.path(1));
      else if (path && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(1, uri.path_length());
      }
      /////////
      if (pair.second.at(7))
        TEST_ASSERT_EQUAL_STRING("c_c_c", uri.path(2));
      else if (path) {
        TEST_ASSERT_EQUAL_INT(2, uri.path_length());
      }
      TEST_ASSERT_EQUAL_STRING(pair.second.at(8) ? "x=1&y=2" : "", uri.query());
      TEST_ASSERT_EQUAL_STRING(pair.second.at(9) ? "fhatty" : "", uri.fragment());
    }
  }

  void test_uri_scheme() {
    TEST_ASSERT_EQUAL_STRING("", UriX("127.0.0.1").scheme());
    TEST_ASSERT_EQUAL_STRING("fos", UriX("fos:person").scheme());
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("person"), UriX("fos:person").scheme(""));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("fos://person@127.0.0.1"), UriX("//person@127.0.0.1").scheme("fos"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("fos:a/b/c"), UriX("a/b/c").scheme("fos"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("fos:b:c"), UriX("a:b:c").scheme("fos"));
    TEST_ASSERT_EQUAL_STRING("fos", UriX("fos://x@127.0.0.1/person").scheme());
    TEST_ASSERT_EQUAL_STRING("x", UriX("fos://x@127.0.0.1/person").user());
    TEST_ASSERT_EQUAL_STRING("x", UriX("fos://x:password@127.0.0.1/person").user());
    TEST_ASSERT_EQUAL_STRING("password", UriX("fos://x:password@127.0.0.1/person").password());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("fos://x@127.0.0.1/person").host());
  }

  void test_uri_user_password() {
    TEST_ASSERT_EQUAL_STRING("fhat", UriX("//fhat@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("", UriX("fhat@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("", UriX("fos://fhat@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("fhat", UriX("//fhat:pig@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("pig", UriX("//fhat:pig@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("pig", UriX("//pig@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("", UriX("fos://pig@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("pass", UriX("fos://pig:pass@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("x", UriX("//x@/a/b/c/d/e").user());
  }

  void test_uri_host() {
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//fhat@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("", UriX("fhat@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("xxx://fhat:pig@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//fhat:pig@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("", UriX("/a/b/c").host());
    TEST_ASSERT_EQUAL_STRING("", UriX("fhat@/a/b/c").host());
    /////
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a"), UriX("/a").host("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b/c"), UriX("/a/b/c").host("127.0.0.1"));
    //  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"), fURI("/").host("127.0.0.1"));
  }

  void test_uri_authority() {
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//127.0.0.1").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//127.0.0.1/a").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//127.0.0.1/a/b").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", UriX("//127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING(UriX("//127.0.0.1/a/b/c").authority().c_str(),
                             UriX("//127.0.0.1/d/e").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("fat@127.0.0.1", UriX("//fat@127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("fat:pig@127.0.0.1", UriX("//fat:pig@127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("", UriX("/a").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("", UriX("x@/a/b/c/d/e").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("x@", UriX("//x@/a/b/c/d/e").authority().c_str());
    //////
    /*FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1/a"), UriX("1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1"), UriX("1.1.1.1").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1/a"), UriX("1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1/a"), UriX("/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1"), UriX("").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("fhat@"), UriX("fhat@127.0.0.1").host(""));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("127.0.0.1"), UriX("fhat@127.0.0.1").user(""));
    TEST_ASSERT_EQUAL_STRING("", UriX("127.0.0.1").host("").toString().c_str());
    TEST_ASSERT_EQUAL_STRING("", UriX("fhat@127.0.0.1").user("").host("").toString().c_str());
    TEST_ASSERT_TRUE(fURI("/a/b/c").authority().empty());*/
  }

  void test_uri_path() {
    TEST_ASSERT_EQUAL_STRING("", UriX("").path().c_str());
    TEST_ASSERT_EQUAL_STRING("", UriX("//127.0.0.1").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a", UriX("//127.0.0.1/a").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c", UriX("//127.0.0.1/a/b/c").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c", UriX("//fhat@127.0.0.1/a/b/c").path().c_str());
    TEST_ASSERT_EQUAL_STRING("b", UriX("furi://fhat@127.0.0.1/a/b/c/d/e").path(1));
    TEST_ASSERT_EQUAL_STRING("/c/d/", UriX("fos://fhat@127.0.0.1/a/b/c/d/e").path(2, 4).c_str());
    TEST_ASSERT_EQUAL_STRING("/e", UriX("fos://fhat@127.0.0.1/a/b/c/d/e").path(4, 5).c_str());
    TEST_ASSERT_EQUAL_STRING("/", UriX("//fhat@127.0.0.1/a/b/c/d/e").path(5, 6).c_str());
    TEST_ASSERT_EQUAL_STRING("", UriX("//fhat@127.0.0.1/a/b/c/d/e").path(6));
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", UriX("/a/b/c/d/e").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", UriX("//x@/a/b/c/d/e").path().c_str());
    //
    TEST_ASSERT_EQUAL_INT(0, UriX("").path_length());
    TEST_ASSERT_EQUAL_INT(0, UriX("foi://fhat@127.0.0.1").path_length());
    TEST_ASSERT_EQUAL_INT(1, UriX("fos://fhat@127.0.0.1/a").path_length());
    TEST_ASSERT_EQUAL_INT(5, UriX("furi://fhat@127.0.0.1/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, UriX("/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, UriX("//x@/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, UriX("a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(4, UriX("//x@a/b/c/d/e").path_length());
    //
    TEST_ASSERT_EQUAL_STRING("a", UriX("//127.0.0.1/a/b/c").path(0));
    TEST_ASSERT_EQUAL_STRING("b", UriX("//127.0.0.1/a/b/c").path(1));
    TEST_ASSERT_EQUAL_STRING("c", UriX("//127.0.0.1/a/b/c").path(2));
    TEST_ASSERT_EQUAL_STRING("", UriX("//127.0.0.1/a/b/c").path(3));
    TEST_ASSERT_EQUAL_STRING("a", UriX("//127.0.0.1///a//b////c").path(2));
    TEST_ASSERT_EQUAL_STRING("", UriX("//127.0.0.1///a//b////c").path(3));
    TEST_ASSERT_EQUAL_STRING("b", UriX("//127.0.0.1///a//b////c").path(4));
    TEST_ASSERT_EQUAL_STRING("c", UriX("//127.0.0.1///a//b////c").path(8));
  }

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  void test_uri_extend() {
    /// TRUE
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a"), UriX("//127.0.0.1").extend("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/"), UriX("//127.0.0.1/a").extend(""));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b"), UriX("//127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//fhat:pig@127.0.0.1/a/b"), UriX("//fhat:pig@127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//fhat:pig@127.0.0.1/a/b"), UriX("//fhat:pig@127.0.0.1").extend("a/").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b"), UriX("//127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b//c"), UriX("//127.0.0.1/a/b").extend("/c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b/c"), UriX("//127.0.0.1/a/b").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b/c"), UriX("//127.0.0.1/a/b/").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b/c/"), UriX("//127.0.0.1/a/b/").extend("c/"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("//127.0.0.1/a/b//c/"), UriX("//127.0.0.1/a/b/").extend("/c/"));
    ///
   // FOS_TEST_ASSERT_EQUAL_FURI(UriX("a/b//c"), UriX("a/b").extend("/c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("a/b/c"), UriX("a/b").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("a/b/c"), UriX("a/b/").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(UriX("/a/b/c/"), UriX("/a/b/").extend("c/"));
  //  FOS_TEST_ASSERT_EQUAL_FURI(UriX("/a/b//c/"), UriX("/a/b/").extend("/c/"));
  }


  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_uri_components); //
      FOS_RUN_TEST(test_uri_scheme); //
      FOS_RUN_TEST(test_uri_user_password); //
      FOS_RUN_TEST(test_uri_host); //
      FOS_RUN_TEST(test_uri_authority); //
      FOS_RUN_TEST(test_uri_path); //
      //
      FOS_RUN_TEST(test_uri_extend); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
