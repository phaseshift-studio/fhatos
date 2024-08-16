
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

#undef FOS_TEST_ON_BOOT
#include <test_fhatos.hpp>
#include <furi.hpp>

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_abc() {
    for (const string &uri:
         List<string>({"//a", "http://127.0.0.1", "http://127.0.0.1/a/b/c", "http://127.0.0.1:80/a/b/c",
                       "furi://a.b.c/a", "furi://a.b.c:8080/a", "furi://user:pass@127.0.0.1/a/b/c", "//127.0.0.1:80",
                       "/int/nat/even", "/int/", "/a/b/c", "furi:a/b/c"})) {
      fURI u = fURI(uri);
      LOG(INFO, "!b%s!!\t!g%s!!\n", uri.c_str(), u.toString().c_str());
      TEST_ASSERT_EQUAL_STRING(uri.c_str(), u.toString().c_str());
    }
  }

  // furi://user:pass@127.0.0.1:88/a/b/c?x=1&y=2#fhatty
  void test_uri_components() {
    // TODO: CAN'T DO FRAGMENTS CAUSE OF MQTT PATTERN MATCHING
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
        //{"furi://127.0.0.1:88/a/bb/c_c_c?x=1&y=2#fhatty", {1, 0, 0, 1, 88, 1, 1, 1, 1, 1}},
        //{"furi://127.0.0.1:88/a/bb#fhatty", {1, 0, 0, 1, 88, 1, 1, 0, 0, 1}},
        {"furi:a", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a/", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"furi:/a/bb", {1, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
        {"a", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"/a", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        {"/a/", {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
        //{"/a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        //{"a#fhatty", {0, 0, 0, 0, 0, 1, 0, 0, 0, 1}},
        //{"a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        //{"/a/bb#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        //{"/a/bb/#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 0, 1}},
        {"a?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        {"/a?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        //{"/a/?x=1&y=2", {0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
        {"/a/bb?x=1&y=2", {0, 0, 0, 0, 0, 1, 1, 0, 1, 0}},
        //{"/a/bb?x=1&y=2#fhatty", {0, 0, 0, 0, 0, 1, 1, 0, 1, 1}},
        //{"/a//c_c_c?x=1&y=2#fhatty", {0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
        //{"furi:/a//c_c_c?x=1&y=2", {1, 0, 0, 0, 0, 1, 0, 1, 1, 0}}
    });

    for (Pair<string, List<int>> pair: uris) {
      TEST_ASSERT_EQUAL_INT(10, pair.second.size());
      fURI uri = fURI(pair.first);
      TEST_ASSERT_EQUAL_STRING(pair.first.c_str(), uri.toString().c_str());
      FOS_TEST_ASSERT_EQUAL_FURI(uri, fURI(uri.toString()));
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

  void test_uri_memory_leaks() {
#ifndef NATIVE
    Ansi<>::singleton()->flush();
    int sketchMemory = -1;
    int heapMemory = -1;
#endif
#ifndef NATIVE
#define FOS_LOOPS 10000
#else
#define FOS_LOOPS 50000
#endif
    for (int i = 0; i < FOS_LOOPS; i++) {
      fURI a = fURI("127.0.0.1");
      fURI b = fURI(a);
      fURI c = fURI(b.toString());
      fURI d = fURI(c.path(0));
      fURI e = fURI(d.path(0)).extend("");
      fURI f = fURI(e.toString());
      TEST_ASSERT_TRUE(a.equals(a));
      TEST_ASSERT_TRUE(a.equals(b));
      TEST_ASSERT_TRUE(a.equals(c));
      TEST_ASSERT_TRUE(a.equals(d));
      TEST_ASSERT_TRUE(a.extend("").equals(e));
      TEST_ASSERT_TRUE(a.equals(f));
#undef FOS_LOOPS
#ifndef NATIVE
      if (sketchMemory != -1) {
        TEST_ASSERT_EQUAL_INT32(sketchMemory, ESP.getFreeSketchSpace());
        TEST_ASSERT_EQUAL_INT32(heapMemory, ESP.getFreeHeap());
      }
      sketchMemory = ESP.getFreeSketchSpace();
      heapMemory = ESP.getFreeHeap();
      if (i % 1000 == 0) {
        FOS_TEST_MESSAGE("fURI count: %i\t[free sketch:%i][free heap:%i]", i, sketchMemory, heapMemory);
        Ansi<>::singleton()->flush();
      }
    }
    FOS_TEST_MESSAGE("FINAL [free sketch:%i][free heap:%i]", ESP.getFreeSketchSpace(), ESP.getFreeHeap());
#define FOS_TEST_PRINTER FOS_DEFAULT_PRINTER
#else
      if (i % 5000 == 0) {
        FOS_TEST_MESSAGE("fURI count: %i\t", i);
      }
    }
#endif
  }

  void test_uri_equals() {
    /// STRING EQUALS
    TEST_ASSERT_EQUAL_STRING("", fURI("").toString().c_str());
    TEST_ASSERT_EQUAL_STRING("a", fURI("a").toString().c_str());
    TEST_ASSERT_EQUAL_STRING("/a", fURI("/a").toString().c_str());
    /// TRUE
    TEST_ASSERT_TRUE(fURI("").equals(fURI("")));
    TEST_ASSERT_TRUE(fURI("127.0.0.1").equals(fURI("127.0.0.1")));
    TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b")));
    TEST_ASSERT_TRUE(fURI("fhat@127.0.0.1/a/b").equals(fURI("fhat@127.0.0.1/a/b")));
    TEST_ASSERT_FALSE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b/")));
    TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b/").equals(fURI("127.0.0.1/a/b/")));
    // TEST_ASSERT_FALSE(fURI("127.0.0.1/a/b/").equals(fURI("127.0.0.1/a/b//"))); // TODO: this should be false
    /// FALSE
    TEST_ASSERT_FALSE(fURI("127.0.0.1").equals(fURI("127.1.1.2")));
    TEST_ASSERT_FALSE(fURI("127.0.0.1/a").equals(fURI("127.0.0.1/b")));
    FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a/b"));
    FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("fhat@127.0.0.1/a"), fURI("pig@127.0.0.1/a"));
  }

  void test_uri_scheme() {
    TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1").scheme());
    TEST_ASSERT_EQUAL_STRING("fos", fURI("fos:person").scheme());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("person"), fURI("fos:person").scheme(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://person@127.0.0.1"), fURI("//person@127.0.0.1").scheme("fos"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos:a/b/c"), fURI("a/b/c").scheme("fos"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos:b:c"), fURI("a:b:c").scheme("fos"));
    TEST_ASSERT_EQUAL_STRING("fos", fURI("fos://x@127.0.0.1/person").scheme());
    TEST_ASSERT_EQUAL_STRING("x", fURI("fos://x@127.0.0.1/person").user());
    TEST_ASSERT_EQUAL_STRING("x", fURI("fos://x:password@127.0.0.1/person").user());
    TEST_ASSERT_EQUAL_STRING("password", fURI("fos://x:password@127.0.0.1/person").password());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("fos://x@127.0.0.1/person").host());
  }

  void test_uri_user_password() {
    TEST_ASSERT_EQUAL_STRING("fhat", fURI("//fhat@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("", fURI("fhat@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("", fURI("fos://fhat@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("fhat", fURI("//fhat:pig@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("pig", fURI("//fhat:pig@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("pig", fURI("//pig@127.0.0.1/a/b").user());
    TEST_ASSERT_EQUAL_STRING("", fURI("fos://pig@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("pass", fURI("fos://pig:pass@127.0.0.1/a/b").password());
    TEST_ASSERT_EQUAL_STRING("x", fURI("//x@/a/b/c/d/e").user());
  }

  void test_uri_host() {
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//fhat@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("", fURI("fhat@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("xxx://fhat:pig@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//fhat:pig@127.0.0.1/a/b").host());
    TEST_ASSERT_EQUAL_STRING("", fURI("/a/b/c").host());
    TEST_ASSERT_EQUAL_STRING("", fURI("fhat@/a/b/c").host());
    /////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("/a").host("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c"), fURI("/a/b/c").host("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/"), fURI("/").host("127.0.0.1"));
    //
  }

  void test_uri_authority() {
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//127.0.0.1").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//127.0.0.1/a").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//127.0.0.1/a/b").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("//127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING(fURI("//127.0.0.1/a/b/c").authority().c_str(),
                             fURI("//127.0.0.1/d/e").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("fat@127.0.0.1", fURI("//fat@127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("fat:pig@127.0.0.1", fURI("//fat:pig@127.0.0.1/a/b/c").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("/a").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("x@/a/b/c/d/e").authority().c_str());
    TEST_ASSERT_EQUAL_STRING("x@", fURI("//x@/a/b/c/d/e").authority().c_str());
    //////
    /*FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"), fURI("1.1.1.1").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"), fURI("").authority("127.0.0.1"));
    */
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("furi://fhat@"), fURI("furi://fhat@127.0.0.1").host(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("furi://127.0.0.1"), fURI("furi://fhat@127.0.0.1").user(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("//127.0.0.1").host(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("furi:"), fURI("furi://fhat@127.0.0.1").user("").host(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("furi://fhat@127.0.0.1").user("").host("").scheme(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//bob@"), fURI("furi://fhat@127.0.0.1").user("").host("").scheme("").user("bob"));
    TEST_ASSERT_TRUE(fURI("/a/b/c").authority().empty());
  }

  void test_uri_path() {
    TEST_ASSERT_EQUAL_STRING("", fURI("").path().c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("//127.0.0.1").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a", fURI("//127.0.0.1/a").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c", fURI("//127.0.0.1/a/b/c").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c", fURI("//fhat@127.0.0.1/a/b/c").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path(0, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path(1));
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").path(2, 4).c_str());
    TEST_ASSERT_EQUAL_STRING("e", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").path(4, 5).c_str());
    TEST_ASSERT_EQUAL_STRING("e/", fURI("fos://fhat@127.0.0.1/a/b/c/d/e/").path(4, 5).c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("//fhat@127.0.0.1/a/b/c/d/e").path(5, 6).c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("//fhat@127.0.0.1/a/b/c/d/e").path(6));
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", fURI("/a/b/c/d/e").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", fURI("//x@/a/b/c/d/e").path().c_str());
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("/a/b/c/d/e").path(2, 4).c_str());
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("//x@/a/b/c/d/e").path(2, 4).c_str());
    //
    TEST_ASSERT_EQUAL_INT(0, fURI("").path_length());
    TEST_ASSERT_EQUAL_INT(0, fURI("foi://fhat@127.0.0.1").path_length());
    TEST_ASSERT_EQUAL_INT(1, fURI("fos://fhat@127.0.0.1/a").path_length());
    TEST_ASSERT_EQUAL_INT(5, fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, fURI("/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, fURI("//x@/a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(5, fURI("a/b/c/d/e").path_length());
    TEST_ASSERT_EQUAL_INT(4, fURI("//x@a/b/c/d/e").path_length());
    //
    TEST_ASSERT_EQUAL_STRING("a", fURI("//127.0.0.1/a/b/c").path(0));
    TEST_ASSERT_EQUAL_STRING("b", fURI("//127.0.0.1/a/b/c").path(1));
    TEST_ASSERT_EQUAL_STRING("c", fURI("//127.0.0.1/a/b/c").path(2));
    TEST_ASSERT_EQUAL_STRING("", fURI("//127.0.0.1/a/b/c").path(3));
    TEST_ASSERT_EQUAL_STRING("a", fURI("//127.0.0.1///a//b////c").path(2));
    TEST_ASSERT_EQUAL_STRING("", fURI("//127.0.0.1///a//b////c").path(3));
    TEST_ASSERT_EQUAL_STRING("b", fURI("//127.0.0.1///a//b////c").path(4));
    TEST_ASSERT_EQUAL_STRING("c", fURI("//127.0.0.1///a//b////c").path(8));
  }

  void test_uri_query() {
    TEST_ASSERT_TRUE(0 == strlen(fURI("127.0.0.1").query()));
    TEST_ASSERT_EQUAL_STRING("testing", fURI("127.0.0.1/a/b?testing").query());
    TEST_ASSERT_EQUAL_STRING("testing=123", fURI("127.0.0.1?testing=123").query());
    TEST_ASSERT_EQUAL_STRING("a=1;b=2", fURI("fhat@127.0.0.1?a=1;b=2").query());
    TEST_ASSERT_EQUAL_STRING("a;b;c", fURI("/a/b/c?a;b;c").query());
    TEST_ASSERT_EQUAL_STRING("query", fURI("127.0.0.1/a?query").query());
    TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1/a?").query());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a?query").query(""));
    TEST_ASSERT_EQUAL_STRING("127.0.0.1/a?testing", fURI("127.0.0.1/a/b?testing").retract().toString().c_str());
    ////////////////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a?a=1"), fURI("127.0.0.1/a").query("a=1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1/a?a=1;b=2;c=3"), fURI("fhat@127.0.0.1/a").query("a=1;b=2;c=3"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a?a=1;b=2;c=3"), fURI("/a").query("a=1;b=2;c=3"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("?a,b,c"), fURI("").query("a,b,c"));
  }

  void test_uri_empty() {
    TEST_ASSERT_TRUE(fURI("").empty());
    TEST_ASSERT_FALSE(fURI("fos:").empty());
    TEST_ASSERT_FALSE(fURI("a/b/c").empty());
    TEST_ASSERT_FALSE(fURI("http://a.com:34/b/c/#").empty());
  }

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  void test_uri_extend() {
    /// TRUE
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("//127.0.0.1").extend("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/"), fURI("//127.0.0.1/a").extend(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b"), fURI("//127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//fhat:pig@127.0.0.1/a/b"), fURI("//fhat:pig@127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//fhat:pig@127.0.0.1/a/b"), fURI("//fhat:pig@127.0.0.1").extend("a/").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b"), fURI("//127.0.0.1").extend("a").extend("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b//c"), fURI("//127.0.0.1/a/b").extend("/c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c"), fURI("//127.0.0.1/a/b").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c"), fURI("//127.0.0.1/a/b/").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c/"), fURI("//127.0.0.1/a/b/").extend("c/"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b//c/"), fURI("//127.0.0.1/a/b/").extend("/c/"));
    ///
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b//c"), fURI("a/b").extend("/c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/b").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/b/").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/c/"), fURI("/a/b/").extend("c/"));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b//c/"), fURI("/a/b/").extend("/c/"));
  }

  void test_uri_resolve() {
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1/").resolve("/a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1").resolve("/a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1/").resolve("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1").resolve("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/b"), fURI("foi://127.0.0.1/").resolve("a").resolve("/b"));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a/"), fURI("foi://127.0.0.1/").resolve("a/"));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a/b"), fURI("foi://127.0.0.1/").resolve("a/").resolve("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/b"), fURI("foi://127.0.0.1/").resolve("a").resolve("b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/"), fURI("foi://127.0.0.1/").resolve(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1"), fURI("foi://127.0.0.1").resolve(""));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/"), fURI("foi://127.0.0.1").resolve("/"));
    ///
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/thread"), fURI("/rec/").resolve("/thread"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/rec/thread"), fURI("/rec/").resolve(fURI("thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/thread"), fURI("rec").resolve(fURI("/thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("thread"), fURI("rec").resolve(fURI("thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/thread"), fURI("/rec").resolve(fURI("/thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/thread"), fURI("/rec").resolve(fURI("thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/rec/thread"), fURI("127.0.0.1/rec/").resolve(fURI("thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/thread"), fURI("127.0.0.1/rec").resolve(fURI("thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/thread"), fURI("//127.0.0.1/rec").resolve(fURI("/thread")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://123.0.0.4/types/int/nat/even"),
                               fURI("foi://123.0.0.4/types/int/nat/").resolve("even"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://123.0.0.4/types/int/even"),
                               fURI("foi://123.0.0.4/types/int/nat").resolve("even"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://123.0.0.4/types/int/even"),
                               fURI("foi://123.0.0.4/types/int/nat/").resolve("../even"));
    ////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/d"), fURI("/a/b/c").resolve(fURI("d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d"), fURI("/a/b/c").resolve(fURI("/d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/d"), fURI("/a/b/c/").resolve(fURI("../d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d"), fURI("/a/b/c").resolve(fURI("../d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d/"), fURI("/a/b/c").resolve(fURI("../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/d/"), fURI("/a/b/c/").resolve(fURI("../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d/"), fURI("/a/b/c").resolve(fURI("../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d/"), fURI("/a/b/c").resolve(fURI("../../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d"), fURI("/a/b/c").resolve(fURI("../../d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d"), fURI("/a/b/c/d").resolve(fURI("../../d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d/"), fURI("/a/b/c/d").resolve(fURI("../../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d/"), fURI("/a/b/c/d").resolve(fURI("../../../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d/"), fURI("/a/b/c/d").resolve(fURI("../.././.././d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d"), fURI("/a/b").resolve(fURI("./d")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d/"), fURI("/a/b").resolve(fURI("./d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/d/"), fURI("/a/b").resolve(fURI("././d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/d/"), fURI("/a/b").resolve(fURI("././../d/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("b"), fURI("a").resolve(fURI("b")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/c"), fURI("a").resolve(fURI("/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("b/c"), fURI("a").resolve(fURI("b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a").resolve(fURI("a")));
    ///////////////// mm-ADT specific :-based resolution
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs:root"), fURI("/inst/fs:").resolve("root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs:root"), fURI("/inst/fs").resolve(":root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs:").resolve(":root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs::").resolve("root"));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs").resolve("::root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://inst/fs:root"), fURI("x://inst/fs").resolve(":root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs:root"), fURI("x://user@inst/fs:").resolve("root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs:root/more"), fURI("x://user@inst/fs:").resolve("root/more"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs::root/more"), fURI("x://user@inst/fs:").resolve(":root/more"));
    /////////////////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://fhat@127.0.0.1/b/c"), fURI("foi://fhat@127.0.0.1/a").resolve(fURI("b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://fhat@127.0.0.1/b/c"),
                               fURI("foi://fhat@127.0.0.1/a").resolve(fURI("foi://fhat@fhat.org/b/c")));
    // TODO:
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/"), fURI("/a/").resolve(fURI("./b/")));
    //     FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/"), fURI("/a/").resolve(fURI("b/")));
  }

  void test_uri_match() {
    //// TRUE
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1"), fURI("+"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1"), fURI("#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("fhat@127.0.0.1/a"), fURI("fhat@127.0.0.1/#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/abc.org"), fURI("127.0.0.1/#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/+/b"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/+/b"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/+/+"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/a/+/c"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/a/+/#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c/d"), fURI("127.0.0.1/a/+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/+/x/v"));
    // TODO: ?? TEST_ASSERT_TRUE(fURI("127.0.0.1"),fURI("127.0.0.1/#"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("fos://127.0.0.1/a"), fURI("fos://127.0.0.1/a"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("fos://127.0.0.1:21/a/b"), fURI("fos://127.0.0.1:21/+/b"));
    //// FALSE
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1"), fURI("127.0.0.2"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.2/?/b"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1"), fURI("127.0.0.1/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/+/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/abc"), fURI("127.0.0.1/abc/#"));
    //
    FOS_TEST_ASSERT_MATCH_FURI(fURI("/type/uri/fs:file"), Pattern("/type/#"));
  }

  void test_fhat_idioms() {
    fURI nat("/int/nat");
    TEST_ASSERT_EQUAL_STRING("nat", nat.name());
    TEST_ASSERT_EQUAL_STRING("/int/nat", nat.toString().c_str());
    TEST_ASSERT_EQUAL_STRING("int", nat.path(0));
    TEST_ASSERT_EQUAL_STRING("nat", nat.path(1));
    TEST_ASSERT_EQUAL_STRING("", nat.path(3));
    TEST_ASSERT_EQUAL_INT(2, nat.path_length());
    TEST_ASSERT_EQUAL_STRING("/", nat.path(0, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("/int", nat.path(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.path(2, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.path(2, 3).c_str());
    TEST_ASSERT_EQUAL_STRING("/int/nat", nat.path(0, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("nat", nat.path(1, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.path(2, 2).c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/int/nat"), ID("/int/").resolve(ID("nat")));
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/int/"), ID("/int/").resolve(ID("")));
  }

  void test_pattern_pattern_matching() {
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/fs/#"), Pattern("#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/fs/mount/#"), Pattern("/fs/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/#"), Pattern("/fs/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/fs/#"), Pattern("/fs/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/fs/+/abc"), Pattern("/fs/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/fs/#"), Pattern("/fs/+/abc"));
    //
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("//#"), Pattern("//+"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("//+"), Pattern("//#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("//#"), Pattern("/+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("//+/#"), Pattern("/fhat/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/fhat/#"), Pattern("//+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("//+/#"), Pattern("/fhat/aus/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/fhat/aus/#"), Pattern("//+/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("//+//a"), Pattern("//+//+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("//+//+"), Pattern("//+//a"));
  }

  void test_composite_mutations() {
    FOS_TEST_ASSERT_MATCH_FURI(fURI("fos://127.0.0.1/a/c/d"),
                               fURI("fos://127.0.0.1/a").extend("b").resolve("b/c").resolve("../c/d"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://127.0.0.1/a/b/c"), fURI("fos://127.0.0.1/a").extend("b").extend("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://127.0.0.1/a/b/c"),
                               fURI("fos://127.0.0.1/a").extend("b").extend("").resolve("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://127.0.0.1/a/b/c"), fURI("fos://127.0.0.1/a").extend("b/").resolve("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://127.0.0.1/a/b/c"),
                               fURI("fos://127.0.0.1/a").extend("").resolve("b").extend("").resolve("c"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://127.0.0.1/a/b/c/d"),
                               fURI("fos://127.0.0.1/a").extend("b").extend("c").extend("d").resolve("./d"));
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/c/d"),
                               fURI(fURI("127.0.0.1/a").extend("b").resolve("b/c").resolve("../c/d").path()));
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_uri_components); //
      FOS_RUN_TEST(test_uri_memory_leaks); //
      FOS_RUN_TEST(test_uri_equals); //
      FOS_RUN_TEST(test_uri_scheme); //
      FOS_RUN_TEST(test_uri_user_password); //
      FOS_RUN_TEST(test_uri_host); //
      FOS_RUN_TEST(test_uri_authority); //
      FOS_RUN_TEST(test_uri_path); //
      FOS_RUN_TEST(test_uri_query); //
      FOS_RUN_TEST(test_uri_empty); //
      //
      FOS_RUN_TEST(test_uri_extend); //
      FOS_RUN_TEST(test_uri_resolve); //
      FOS_RUN_TEST(test_uri_match); //
      //
      FOS_RUN_TEST(test_fhat_idioms); //
      FOS_RUN_TEST(test_pattern_pattern_matching); //
      FOS_RUN_TEST(test_composite_mutations); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
