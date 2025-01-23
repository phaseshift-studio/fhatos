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

#ifndef fhatos_test_furi_cpp
#define fhatos_test_furi_cpp

/*#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPES
#define FOS_DEPLOY_SHARED_MEMORY*/
#include "../../src/furi.hpp"
#include "../test_fhatos.hpp"

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  // furi://user:pass@127.0.0.1:88/a/b/c?x=1&y=2#fhatty
  void test_uri_components() {
    // TODO: CAN'T DO FRAGMENTS CAUSE OF MQTT PATTERN MATCHING
    const List<Pair<string, List<int>>> uris = List<Pair<string, List<int>>>({
        {"furi:", {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {"furi://127.0.0.1", {1, 0, 0, 1, 0, 0, 0, 0, 0, 0}},
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
        TEST_ASSERT_EQUAL_STRING("a", uri.segment(0));
      else if (pair.second.at(6) == 0 && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(0, uri.path_length());
      }
      /////////
      if (pair.second.at(6))
        TEST_ASSERT_EQUAL_STRING("bb", uri.segment(1));
      else if (path && pair.second.at(7) == 0) {
        path = false;
        TEST_ASSERT_EQUAL_INT(1, uri.path_length());
      }
      /////////
      if (pair.second.at(7))
        TEST_ASSERT_EQUAL_STRING("c_c_c", uri.segment(2));
      else if (path) {
        TEST_ASSERT_EQUAL_INT(2, uri.path_length());
      }
      TEST_ASSERT_EQUAL_STRING(pair.second.at(8) ? "x=1&y=2" : "", uri.query());
      // TEST_ASSERT_EQUAL_STRING(pair.second.at(9) ? "fhatty" : "", uri.fragment());
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
      auto a = fURI("127.0.0.1");
      auto b = fURI(a);
      auto c = fURI(b.toString());
      auto d = fURI(c.segment(0));
      auto e = fURI(d.segment(0)).extend("");
      auto f = fURI(e.toString());
      auto g = fURI("127.0.0.1/");
      if (!a.equals(a)) {
        FOS_TEST_ASSERT_EQUAL_FURI(a, a);
      }
      if (!a.equals(b)) {
        FOS_TEST_ASSERT_EQUAL_FURI(a, b);
      }
      if (!a.equals(c)) {
        FOS_TEST_ASSERT_EQUAL_FURI(a, c);
      }
      if (!a.equals(d)) {
        FOS_TEST_ASSERT_EQUAL_FURI(a, d);
      }
      if (!a.extend("").equals(e)) {
        FOS_TEST_ASSERT_EQUAL_FURI(a.extend(""), e);
      }
      if (!g.equals(e)) {
        FOS_TEST_ASSERT_EQUAL_FURI(g, e);
      }
      if (!g.equals(f)) {
        FOS_TEST_ASSERT_EQUAL_FURI(g, f);
      }
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
    //
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("aaa_bbb/ddd"),  fURI("aaa_bbb/ccc/../ddd"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("aaa_bbb/ddd/"),  fURI("aaa_bbb/ccc/../ddd/"));
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
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1"), fURI("//1.1.1.1").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("//1.1.1.1/a").authority("//127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/1.1.1.1/a"), fURI("1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("//1.1.1.1/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("/a").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("a").authority("//127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("a").authority("//127.0.0.1/"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/"), fURI("/a/").authority("//127.0.0.1/"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//a/a"), fURI("a").authority("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1"), fURI("").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//fhat:os@127.0.0.1/a/"), fURI("/a/").authority("//fhat:os@127.0.0.1/"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//fhat:os@127.0.0.1/a/b/c"), fURI("a/b/c").authority("fhat:os@127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("").authority(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("ftp://127.0.0.1"), fURI("ftp://localhost:10").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("ftp://127.0.0.1/abc"), fURI("ftp://localhost:10/abc").authority("127.0.0.1"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("furi:"), fURI("furi://fhat@127.0.0.1").authority(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//"), fURI("//fhat@127.0.0.1").authority(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/c"), fURI("//fhat@127.0.0.1/a/b/c").authority(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("furi:/a/b/c"), fURI("furi://fhat@127.0.0.1/a/b/c").authority(""));
    /////
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
    TEST_ASSERT_EQUAL_STRING("/a/b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").subpath(0, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").segment(1));
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").subpath(2, 4).c_str());
    TEST_ASSERT_EQUAL_STRING("e", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").subpath(4, 5).c_str());
    TEST_ASSERT_EQUAL_STRING("e/", fURI("fos://fhat@127.0.0.1/a/b/c/d/e/").subpath(4, 5).c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("//fhat@127.0.0.1/a/b/c/d/e").subpath(5, 6).c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("//fhat@127.0.0.1/a/b/c/d/e").segment(6));
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", fURI("/a/b/c/d/e").path().c_str());
    TEST_ASSERT_EQUAL_STRING("/a/b/c/d/e", fURI("//x@/a/b/c/d/e").path().c_str());
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("/a/b/c/d/e").subpath(2, 4).c_str());
    TEST_ASSERT_EQUAL_STRING("c/d", fURI("//x@/a/b/c/d/e").subpath(2, 4).c_str());
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
    TEST_ASSERT_EQUAL_STRING("a", fURI("//127.0.0.1/a/b/c").segment(0));
    TEST_ASSERT_EQUAL_STRING("b", fURI("//127.0.0.1/a/b/c").segment(1));
    TEST_ASSERT_EQUAL_STRING("c", fURI("//127.0.0.1/a/b/c").segment(2));
    TEST_ASSERT_EQUAL_STRING("", fURI("//127.0.0.1/a/b/c").segment(3));
    TEST_ASSERT_EQUAL_STRING("a", fURI("//127.0.0.1///a//b////c").segment(2));
    TEST_ASSERT_EQUAL_STRING("", fURI("//127.0.0.1///a//b////c").segment(3));
    TEST_ASSERT_EQUAL_STRING("b", fURI("//127.0.0.1///a//b////c").segment(4));
    TEST_ASSERT_EQUAL_STRING("c", fURI("//127.0.0.1///a//b////c").segment(8));
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
    TEST_ASSERT_EQUAL_STRING("1,2,3",fURI("x?a=1,2,3&b=4").query_value("a").value().c_str());
    TEST_ASSERT_EQUAL_STRING("4",fURI("x?a=1,2,3&b=4").query_value("b").value().c_str());
    List<string> values = fURI("x?a=1,2,3&b=4").query_values("a");
    TEST_ASSERT_EQUAL_INT(3,values.size());
    TEST_ASSERT_EQUAL_STRING("1",values.at(0).c_str());
    TEST_ASSERT_EQUAL_STRING("2",values.at(1).c_str());
//  TEST_ASSERT_EQUAL_STRING("3",values.at(2).c_str());
     values = fURI("x?a=1,2,3&b=4").query_values("b");
    TEST_ASSERT_EQUAL_INT(1,values.size());
 //   TEST_ASSERT_EQUAL_STRING("4",values.at(0).c_str());
    ////////////////
    TEST_ASSERT_TRUE(fURI("127.0.0.1/a?a=1").has_query());
    TEST_ASSERT_FALSE(fURI("127.0.0.1/a?").has_query());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a?"));
    TEST_ASSERT_TRUE(fURI("127.0.0.1/?sub").has_query());
    TEST_ASSERT_TRUE(fURI("fos://127.0.0.1/?sub").has_query());
    TEST_ASSERT_FALSE(fURI("fos://127.0.0.1/sub").has_query());
    /////////////////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//x/y"), fURI("//x/y?sub").query(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//x/y/"), fURI("//x/y/?sub").query(""));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//x/y/?sub"), fURI(fURI("//x/y/?sub").toString()));
    /////////////////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("abc"), fURI("abc").no_query());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("abc"), fURI("abc?a=2&b=3").no_query());
    FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("?a=2&b=3").no_query());
    TEST_ASSERT_TRUE(fURI("?a=2&b=3").no_query().empty());
  }

  void test_uri_query_value() {
    TEST_ASSERT_FALSE(fURI("foi://127.0.0.1?").query_value("x").has_value());
    TEST_ASSERT_FALSE(fURI("foi://127.0.0.1?z=123").query_value("x").has_value());
    TEST_ASSERT_FALSE(fURI("foi://127.0.0.1?z").query_value("x").has_value());
    TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1?x").query_value("x").value().c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1?y=abc&x").query_value("x").value().c_str());
    TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1?x&y=abc").query_value("x").value().c_str());
    TEST_ASSERT_EQUAL_STRING("123", fURI("127.0.0.1?x=123").query_value("x").value().c_str());
    TEST_ASSERT_EQUAL_STRING("123", fURI("127.0.0.1?y=abc&x=123").query_value("x").value().c_str());
    TEST_ASSERT_EQUAL_STRING("123", fURI("127.0.0.1?x=123&y=abc").query_value("x").value().c_str());
  }

  void test_uri_name() {
    TEST_ASSERT_EQUAL_STRING("", fURI("").name().c_str());
    TEST_ASSERT_EQUAL_STRING("fhat", fURI("fos://a/fhat").name().c_str());
    TEST_ASSERT_EQUAL_STRING("fs:root", fURI("/type/inst/fs:root").name().c_str());
    TEST_ASSERT_EQUAL_STRING("root", fURI("/type/inst/fs::root").name().c_str());
    TEST_ASSERT_EQUAL_STRING("#", fURI("http://a.com:34/b/c/#").name().c_str());
    TEST_ASSERT_EQUAL_STRING("c:fhatty", fURI("http://a.com:34/b/c:fhatty").name().c_str());
    TEST_ASSERT_EQUAL_STRING(":fhatty", fURI("fos/:fhatty").name().c_str());
  }

  void test_uri_scheme_path() {
    TEST_ASSERT_FALSE(fURI(":root").is_scheme_path());
    TEST_ASSERT_TRUE(fURI("fs:root").is_scheme_path());
    TEST_ASSERT_TRUE(fURI("fs:/root").is_scheme_path());
    TEST_ASSERT_TRUE(fURI("fs:/root/").is_scheme_path());
    TEST_ASSERT_FALSE(fURI("fs://fhatos/root").is_scheme_path());
    TEST_ASSERT_FALSE(fURI("fs://fhatos/").is_scheme_path());
    TEST_ASSERT_FALSE(fURI("fs://user:pass@fhatos/").is_scheme_path());
  }

  void test_uri_colon_path() {
  TEST_ASSERT_EQUAL_STRING("/mmadt/int/::/zero",fURI("/mmadt/int::zero").toString().c_str());
  TEST_ASSERT_EQUAL_STRING("",fURI("/sys/scheduler/:stop").scheme());
  TEST_ASSERT_EQUAL_STRING("",fURI("/sys/scheduler/:stop").host());
  TEST_ASSERT_EQUAL_STRING("",fURI("/sys/scheduler/:stop").user());
  TEST_ASSERT_EQUAL_STRING("",fURI("/sys/scheduler/:stop").password());
  TEST_ASSERT_EQUAL_STRING("/sys/scheduler/:stop",fURI("/sys/scheduler/:stop").path().c_str());
  TEST_ASSERT_EQUAL_STRING(":stop",fURI("/sys/scheduler/:stop").name().c_str());
  //TEST_ASSERT_EQUAL_STRING("/mmadt/int//::/zero",fURI("/mmadt/int/::zero").toString().c_str());
  //TEST_ASSERT_EQUAL_STRING("/mmadt/int//:://zero",fURI("/mmadt/int/::/zero").toString().c_str());
  //TEST_ASSERT_EQUAL_STRING("/mmadt/int/:://zero",fURI("/mmadt/int::/zero").toString().c_str());
  //FOS_TEST_ASSERT_EQUAL_FURI(fURI("/mmadt/int//::/zero"),fURI("/mmadt/int/").extend("::zero"));
  //FOS_TEST_ASSERT_EQUAL_FURI(fURI("/mmadt/int/::/zero"),fURI("/mmadt/int").resolve("::zero"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/mmadt/int/::/zero"),fURI("/mmadt/int::zero"));
  //FOS_TEST_ASSERT_EQUAL_FURI(fURI("/mmadt/int//::/zero"),fURI("/mmadt/int/::zero"));
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
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c"), fURI("//127.0.0.1/a/b").extend("/c"));
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
    /////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("abc/:loop"), fURI("abc").extend(":loop"));
  }

void test_uri_prepend() {
  /// TRUE
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("b/a"), fURI("a").prepend("b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a"), fURI("/a").prepend("b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a"), fURI("a").prepend("/b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a"), fURI("/a").prepend("/b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a/"), fURI("/a/").prepend("/b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a/"), fURI("/a/").prepend("/b/"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a"), fURI("a").prepend("/b/"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/c/b/a"), fURI("/a").prepend("/b").prepend("c"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("c/b/a"), fURI("a").prepend("b").prepend("c"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/c/b/a/"), fURI("/a/").prepend("/b/").prepend("/c/"));
}

void test_uri_retract() {
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/"), fURI("a/b/c/").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b"), fURI("a/b/c").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/"), fURI("a/b/c/").retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a/b/c").retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a/b/c/#").retract().retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("a/b/c/d").retract().retract().retract().retract());
  //
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/::/c/"), fURI("a/b/::/c/d/").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/::/c"), fURI("a/b/::/c/d").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/"), fURI("a/b/::/c/d/").retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b"), fURI("a/b/::/c/d").retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a/b/::/c/d").retract().retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("a/b/::/c/d/").retract().retract().retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("a/b/::/c/d").retract().retract().retract().retract());
}

void test_uri_retract_pattern() {
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c/"), fURI("a/b/c/").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/b/c").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/b/c/#").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c/"), fURI("a/b/c/#/").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b"), fURI("a/b/+/#").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/"), fURI("a/b/+/#/").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a/+/c/#").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/"), fURI("a/+/c/#/").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("").retract_pattern());
  //////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a/b/c"), fURI("mqtt://a/b/c/+").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a:8080/b/c/"), fURI("mqtt://a:8080/b/c/+/").retract_pattern());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a:8080/b/c"), fURI("mqtt://a:8080/b/c/+").retract_pattern());
  }


  void test_uri_remove_subpath() {
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a/"), fURI("mqtt://a/b/c").remove_subpath("b/c",false));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a"), fURI("mqtt://a/b/c").remove_subpath("/b/c",false));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt:///b/c"), fURI("mqtt://a/b/c").remove_subpath("a",false));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://b/c"), fURI("mqtt://a/b/c").remove_subpath("a/",false));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a/c"), fURI("mqtt://a/b/c").remove_subpath("b/",false));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt:///b/c"), fURI("mqtt://a/b/c").remove_subpath("a",false));
    /// FORWARD
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/c"), fURI("mqtt://a/b/c").remove_subpath("mqtt://a/b",true));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a"), fURI("mqtt://a/b/c").remove_subpath("/b/c",true));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt:///b/c"), fURI("mqtt://a/b/c").remove_subpath("a",true));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("mqtt://a/c"), fURI("mqtt://a/b/c").remove_subpath("b/",true));
  }

  void test_uri_is_relative() {
    TEST_ASSERT_TRUE(fURI("./").is_relative());
    TEST_ASSERT_TRUE(fURI("./abc/cd").is_relative());
    TEST_ASSERT_TRUE(fURI("././abc").is_relative());
    TEST_ASSERT_TRUE(fURI("../abc").is_relative());
    TEST_ASSERT_TRUE(fURI(".././abc").is_relative());
    TEST_ASSERT_TRUE(fURI(":abc").is_relative());
    TEST_ASSERT_TRUE(fURI(":/abc").is_relative());
    TEST_ASSERT_TRUE(fURI("./:abc").is_relative());
    TEST_ASSERT_TRUE(fURI(":./abc").is_relative());
    TEST_ASSERT_TRUE(fURI(":../abc").is_relative());
    //
    TEST_ASSERT_FALSE(fURI("abc/").is_relative());
    TEST_ASSERT_FALSE(fURI("/abc/cd").is_relative());
    TEST_ASSERT_FALSE(fURI("fos://localhost/./abc").is_relative());
    TEST_ASSERT_FALSE(fURI("fos:./").is_relative());
   	FOS_TEST_EXCEPTION_CXX(fURI("//localhost:8080/="));
  }

  void test_uri_branch_node() {
    TEST_ASSERT_TRUE(fURI("/").is_branch());
    TEST_ASSERT_TRUE(fURI("//abc/").is_branch());
    TEST_ASSERT_TRUE(fURI("fos://abc/").is_branch());
    TEST_ASSERT_TRUE(fURI("/abc/").is_branch());
    TEST_ASSERT_TRUE(fURI("/abc/def/").is_branch());
    TEST_ASSERT_TRUE(fURI("//abc/def//").is_branch());
    TEST_ASSERT_TRUE(fURI("../def/").is_branch());
    ///
    TEST_ASSERT_TRUE(fURI("").is_node());
    TEST_ASSERT_TRUE(fURI("//abc").is_node());
    TEST_ASSERT_TRUE(fURI("furi://a").is_node());
    TEST_ASSERT_TRUE(fURI("furi://a/b/c").is_node());
    TEST_ASSERT_TRUE(fURI("/abc").is_node());
    TEST_ASSERT_TRUE(fURI("/abc/def").is_node());
    TEST_ASSERT_TRUE(fURI("//abc/def").is_node());
    TEST_ASSERT_TRUE(fURI("./def").is_node());
  }

  void test_uri_resolve() {
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1/").resolve("/a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1").resolve("/a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1/").resolve("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a"), fURI("foi://127.0.0.1").resolve("a"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/b"), fURI("foi://127.0.0.1/").resolve("a").resolve("/b"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a/"), fURI("foi://127.0.0.1/").resolve("a/"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://127.0.0.1/a/b"), fURI("foi://127.0.0.1/").resolve("a/").resolve("b"));
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
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/x"), fURI("/a/b/c").resolve("x"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/x"), fURI("/a/").resolve("./x"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/x/y"), fURI("/a/b/c").resolve("../x/y"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/x/y"), fURI("/a/b/c").resolve("../../x/y"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/x/y"), fURI("/a/b/c").resolve("./../x/y"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/x/y"), fURI("/a//b/c").resolve("./../x/y"))
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a//x/y"), fURI("/a//b/c").resolve("../x/y"))
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
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a"), fURI("/a/b/c/d").resolve(fURI("../..")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/"), fURI("/a/b/c/d").resolve(fURI("../../")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/e"), fURI("/a/b/c/d").resolve(fURI("../../e")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/e/"), fURI("/a/b/c/d").resolve(fURI("../../e/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("b"), fURI("a").resolve(fURI("b")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/c"), fURI("a").resolve(fURI("/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("b/c"), fURI("a").resolve(fURI("b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("a").resolve(fURI("a")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/c/b"), fURI("a/c/").resolve(fURI("./b")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b"), fURI("a/c").resolve(fURI("./b")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b"), fURI("a/").resolve(fURI("./b")));
    ///////////////// mm-ADT specific :-based resolution
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs:/type/inst/root"), fURI("fs:/type/inst/").resolve("fs:root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs://type/inst/root"), fURI("fs://type/inst/").resolve("fs:root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs://type/root"), fURI("fs://type/inst/").resolve("fs:../root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/type/inst/fs:root"), fURI("/type/inst/").resolve("fs:root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs:/root"), fURI("/type/inst/").resolve("fs:/root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs:/root"), fURI("fs:/type/inst/").resolve("fs:/root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("fs://localhost/root"), fURI("fs://localhost/inst/").resolve("fs:/root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("xx://localhost/inst/fs:/root"), fURI("xx://localhost/inst/").resolve("fs:/root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("xx://localhost/inst/fs:root"), fURI("xx://localhost/inst/").resolve("fs:root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs:root"), fURI("/inst/fs:").resolve("root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs:root"), fURI("/inst/fs").resolve(":root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs:").resolve(":root"));
    //FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs::").resolve("root"));
    // FOS_TEST_ASSERT_EQUAL_FURI(fURI("/inst/fs::root"), fURI("/inst/fs").resolve("::root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://inst/fs:root"), fURI("x://inst/fs").resolve(":root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs:root"), fURI("x://user@inst/fs:").resolve("root"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs:root/more"), fURI("x://user@inst/fs:").resolve("root/more"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("x://user@inst/fs::root/more"), fURI("x://user@inst/fs:").resolve(":root/more"));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("abc:loop"),fURI("abc").resolve(":loop"));
    /////////////////
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://fhat@127.0.0.1/b/c"), fURI("foi://fhat@127.0.0.1/a").resolve(fURI("b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("foi://fhat@127.0.0.1/b/c"),
                               fURI("foi://fhat@127.0.0.1/a").resolve(fURI("foi://fhat@fhat.org/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/"), fURI("a/").resolve(fURI("./b/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/"), fURI("/a/").resolve(fURI("./b/")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/"), fURI("/a/").resolve(fURI("b/")));

    ////////// PATTERN RESOLUTIONS
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a"), fURI("#").resolve(fURI("a")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/c"), fURI("/a/+/c").resolve(fURI("/a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/+/c").resolve(fURI("a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("a/b/c"), fURI("a/+/c/#").resolve(fURI("a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/c"), fURI("/a/#").resolve(fURI("/a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a/b/c"), fURI("/b/#").resolve(fURI("/a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("/b/a/b/c"), fURI("/b/#").resolve(fURI("a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//b/+/+/a/b/c"), fURI("//b/+/+/").resolve(fURI("a/b/c")));
    FOS_TEST_ASSERT_EQUAL_FURI(fURI("//b/+/a/b/c"), fURI("//b/+/+").resolve(fURI("a/b/c")));
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
    FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/abc"), fURI("127.0.0.1/abc/#"));
    //
    FOS_TEST_ASSERT_MATCH_FURI(fURI("/type/uri/fs:file"), Pattern("/type/#"));
    //// EMPTY PATH SEGMENTS WITH WILDCARDS (MQTT BEHAVIOR TESTED AGAINST MOSQUITTO)
    FOS_TEST_ASSERT_MATCH_FURI(ID("/type/inst/abc"), Pattern("/type/inst/#"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("/type/inst/"), Pattern("/type/inst/#"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("/type/inst"), Pattern("/type/inst/#")); // different than +
    FOS_TEST_ASSERT_MATCH_FURI(ID("/type/inst/abc"), Pattern("/type/inst/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/type/inst/"), Pattern("/type/inst/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/type/inst"), Pattern("/type/inst/+")); // different than #
    ///////////////////////////////////
    FOS_TEST_ASSERT_MATCH_FURI(ID("/fhatos/:name"), ID(":name"));
    FOS_TEST_ASSERT_MATCH_FURI(ID(":name"), ID(":name"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("fos:name"), ID(":name"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("name"), ID(":name"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("aaa/fos:name"), ID(":name"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("name"), ID("fos:name"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("fos:name"), ID("xyz:name"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("fos:name"), ID(":name2"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("fos:name2"), ID(":name"));
    ///////////////////////////////////
    FOS_TEST_ASSERT_MATCH_FURI(ID("/soc/pin/1"), Pattern("/soc/pin/+"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("/soc/pin/1"), Pattern("/soc/pin/#"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("/soc/pin/1/"), Pattern("/soc/pin/+/"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/soc/pin/1"), Pattern("/soc/pin/+/"));
    ///////////////////////////////////
    FOS_TEST_ASSERT_MATCH_FURI(ID("/abc/"), Pattern("/+/"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("x/#"), ID("x/"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("x/"), Pattern("x/#"));
    FOS_TEST_ASSERT_MATCH_FURI(ID("x/"), Pattern("+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("+/#"), ID("x/"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("x/"), Pattern("+/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("+/+"), ID("x/"));
    ///////////////////////////////////
    FOS_TEST_ASSERT_MATCH_FURI(ID("/a/b/c?k"), ID("/a/b/c?k"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/a/b/c?k"), ID("/a/b/c?v"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/a/b/c"), ID("/a/b/c?k"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/a/b/c?k"), ID("/a/b/c"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("/a/b/c?k"), Pattern("/a/b/c?+"));
    //FOS_TEST_ASSERT_NOT_MATCH_FURI(ID("/a/b/c?k"), Pattern("/a/b/c?+/d"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("/a/b/c?k/d"), Pattern("/a/b/c?+/d"));
    /////
  FOS_TEST_ASSERT_MATCH_FURI(ID("z?sub"), Pattern("+"));
    //FOS_TEST_ASSERT_MATCH_FURI(ID("/a/b/c?k"), ID("a/b/#"));
  }

  void test_fhat_idioms() {
    fURI nat("/int/nat");
    TEST_ASSERT_EQUAL_STRING("nat", nat.name().c_str());
    TEST_ASSERT_EQUAL_STRING("/int/nat", nat.toString().c_str());
    TEST_ASSERT_EQUAL_STRING("int", nat.segment(0));
    TEST_ASSERT_EQUAL_STRING("nat", nat.segment(1));
    TEST_ASSERT_EQUAL_STRING("", nat.segment(3));
    TEST_ASSERT_EQUAL_INT(2, nat.path_length());
    TEST_ASSERT_EQUAL_STRING("/", nat.subpath(0, 0).c_str());
    TEST_ASSERT_EQUAL_STRING("/int", nat.subpath(0, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.subpath(2, 1).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.subpath(2, 3).c_str());
    TEST_ASSERT_EQUAL_STRING("/int/nat", nat.subpath(0, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("nat", nat.subpath(1, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("", nat.subpath(2, 2).c_str());
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
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/#"), Pattern("/a/b"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/b"), Pattern("/a/#"));
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
    //
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("a/b"), Pattern("/a/+"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+"), Pattern("/a/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/b"), Pattern("/a/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/+"), Pattern("/a/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/+/+"), Pattern("/a/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/+/+"), Pattern("/a/+/+/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/b/+"), Pattern("/a/+/+/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/b/c/d"), Pattern("/a/+/+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/#"), Pattern("/a/+"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/+"), Pattern("/a/b/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/+"), Pattern("/a/b/c/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/#"), Pattern("/a/b/c/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/#"), Pattern("/a/b/c/+"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/b/+/d"), Pattern("/a/+/+/+"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/b/+/d"), Pattern("/a/+/+/d"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/b/+/d"), Pattern("/a/+/+/e"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/+/+/d"), Pattern("/a/b/+/d"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/+/d"), Pattern("/a/+/+/d"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a/+/+/#"), Pattern("/a/+/+/d"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/+/+/d"), Pattern("/a/+/#"));
    FOS_TEST_ASSERT_MATCH_FURI(Pattern("/a/b"), Pattern("/a/+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("/a"), Pattern("/a/+/#"));
    FOS_TEST_ASSERT_NOT_MATCH_FURI(Pattern("a"), Pattern("/a/+/#"));
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
      FOS_RUN_TEST(test_uri_query_value); //
      FOS_RUN_TEST(test_uri_scheme_path); //
      FOS_RUN_TEST(test_uri_colon_path); //
      FOS_RUN_TEST(test_uri_empty); //
      FOS_RUN_TEST(test_uri_name); //
      //
      FOS_RUN_TEST(test_uri_extend); //
      FOS_RUN_TEST(test_uri_prepend); //
      FOS_RUN_TEST(test_uri_retract);
      FOS_RUN_TEST(test_uri_retract_pattern);
      FOS_RUN_TEST(test_uri_remove_subpath);
      FOS_RUN_TEST(test_uri_is_relative); //
      FOS_RUN_TEST(test_uri_branch_node);
      FOS_RUN_TEST(test_uri_resolve); //
      FOS_RUN_TEST(test_uri_match); //
      //
      FOS_RUN_TEST(test_fhat_idioms); //
      FOS_RUN_TEST(test_pattern_pattern_matching); //
      FOS_RUN_TEST(test_composite_mutations); //
      )
} // namespace fhatos

SETUP_AND_LOOP();

#endif