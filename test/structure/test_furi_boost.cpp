//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once
#ifndef fhatos_test_furi_boost_cpp
#define fhatos_test_furi_boost_cpp
#define BOOST_TEST_MODULE test_furi_boost
#include <boost/test/unit_test.hpp>
#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/ansi.hpp>
string message = string();
#define FOS_TEST_ASSERT_EQUAL_FURI(x, y)                                                                               \
  message.clear();                                                                                                     \
  Ansi<StringPrinter>(&message).printf("<!b%s!!> =!r?!!= <!b%s!!> (%i !rchar_length!! %i) (%i !rpath_length!! %i)",    \
                                       (x).toString().c_str(), (y).toString().c_str(), (x).toString().length(),        \
                                       (y).toString().length(), (x).path_length(), (y).path_length());                 \
  BOOST_TEST_MESSAGE(message);                                                                                         \
  BOOST_ASSERT((x).equals(y));                                                                                         \
  BOOST_ASSERT((x) == (y));                                                                                            \
  BOOST_ASSERT((x).toString() == (y).toString());

#define FOS_TEST_ASSERT_MATCH_FURI(x, y) BOOST_ASSERT((x).matches(y))

#define FOS_TEST_ASSERT_NOT_MATCH_FURI(x, y) BOOST_ASSERT(!(x).matches(y))




using namespace fhatos;
BOOST_AUTO_TEST_SUITE(test_furi_boost_suite)

BOOST_AUTO_TEST_CASE(test_uri_memory_leaks) {
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
    BOOST_ASSERT(a.equals(a));
    BOOST_ASSERT(a.equals(b));
    BOOST_ASSERT(a.equals(c));
    BOOST_ASSERT(a.equals(d));
    BOOST_ASSERT(a.extend("").equals(e));
    BOOST_ASSERT(a.equals(f));
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
      BOOST_TEST_MESSAGE(string("\tfURI count: ") + to_string(i));
    }
  }
#endif
}

BOOST_AUTO_TEST_CASE(test_uri_components) {
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
    BOOST_CHECK_EQUAL(10, pair.second.size());
    fURI uri = fURI(pair.first);
    BOOST_CHECK_EQUAL(pair.first.c_str(), uri.toString().c_str());
    FOS_TEST_ASSERT_EQUAL_FURI(uri, fURI(uri.toString()));
    BOOST_CHECK_EQUAL(pair.second.at(0) ? "furi" : "", uri.scheme());
    BOOST_CHECK_EQUAL(pair.second.at(1) ? "user" : "", uri.user());
    BOOST_CHECK_EQUAL(pair.second.at(2) ? "pass" : "", uri.password());
    BOOST_CHECK_EQUAL(pair.second.at(3) ? "127.0.0.1" : "", uri.host());
    BOOST_CHECK_EQUAL(pair.second.at(4) ? 88 : 0, uri.port());
    bool path = true;
    if (pair.second.at(5))
      BOOST_CHECK_EQUAL("a", uri.path(0));
    else if (pair.second.at(6) == 0 && pair.second.at(7) == 0) {
      path = false;
      BOOST_CHECK_EQUAL(0, uri.path_length());
    }
    /////////
    if (pair.second.at(6))
      BOOST_CHECK_EQUAL("bb", uri.path(1));
    else if (path && pair.second.at(7) == 0) {
      path = false;
      BOOST_CHECK_EQUAL(1, uri.path_length());
    }
    /////////
    if (pair.second.at(7))
      BOOST_CHECK_EQUAL("c_c_c", uri.path(2));
    else if (path) {
      BOOST_CHECK_EQUAL(2, uri.path_length());
    }
    BOOST_CHECK_EQUAL(pair.second.at(8) ? "x=1&y=2" : "", uri.query());
    BOOST_CHECK_EQUAL(pair.second.at(9) ? "fhatty" : "", uri.fragment());
  }
};

BOOST_AUTO_TEST_CASE(test_uri_equals) {
  /// STRING EQUALS
  BOOST_CHECK_EQUAL("", fURI("").toString().c_str());
  BOOST_CHECK_EQUAL("a", fURI("a").toString().c_str());
  BOOST_CHECK_EQUAL("/a", fURI("/a").toString().c_str());
  /// TRUE
  BOOST_ASSERT(fURI("").equals(fURI("")));
  BOOST_ASSERT(fURI("127.0.0.1").equals(fURI("127.0.0.1")));
  BOOST_ASSERT(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b")));
  BOOST_ASSERT(fURI("fhat@127.0.0.1/a/b").equals(fURI("fhat@127.0.0.1/a/b")));
  BOOST_ASSERT(!fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b/")));
  BOOST_ASSERT(fURI("127.0.0.1/a/b/").equals(fURI("127.0.0.1/a/b/")));
  // TEST_ASSERT_FALSE(fURI("127.0.0.1/a/b/").equals(fURI("127.0.0.1/a/b//"))); // TODO: this should be false
  /// FALSE
  BOOST_ASSERT(!fURI("127.0.0.1").equals(fURI("127.1.1.2")));
  BOOST_ASSERT(!fURI("127.0.0.1/a").equals(fURI("127.0.0.1/b")));
  // FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a/b"));
  // FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("fhat@127.0.0.1/a"), fURI("pig@127.0.0.1/a"));
};

BOOST_AUTO_TEST_CASE(test_uri_scheme) {
  BOOST_CHECK_EQUAL("", fURI("127.0.0.1").scheme());
  BOOST_CHECK_EQUAL("fos", fURI("fos:person").scheme());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("person"), fURI("fos:person").scheme(""));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos://person@127.0.0.1"), fURI("//person@127.0.0.1").scheme("fos"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos:a/b/c"), fURI("a/b/c").scheme("fos"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fos:b:c"), fURI("a:b:c").scheme("fos"));
  BOOST_CHECK_EQUAL("fos", fURI("fos://x@127.0.0.1/person").scheme());
  BOOST_CHECK_EQUAL("x", fURI("fos://x@127.0.0.1/person").user());
  BOOST_CHECK_EQUAL("x", fURI("fos://x:password@127.0.0.1/person").user());
  BOOST_CHECK_EQUAL("password", fURI("fos://x:password@127.0.0.1/person").password());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("fos://x@127.0.0.1/person").host());
};

BOOST_AUTO_TEST_CASE(test_uri_user_password) {
  BOOST_CHECK_EQUAL("fhat", fURI("//fhat@127.0.0.1/a/b").user());
  BOOST_CHECK_EQUAL("", fURI("fhat@127.0.0.1/a/b").password());
  BOOST_CHECK_EQUAL("", fURI("fos://fhat@127.0.0.1/a/b").password());
  BOOST_CHECK_EQUAL("fhat", fURI("//fhat:pig@127.0.0.1/a/b").user());
  BOOST_CHECK_EQUAL("pig", fURI("//fhat:pig@127.0.0.1/a/b").password());
  BOOST_CHECK_EQUAL("pig", fURI("//pig@127.0.0.1/a/b").user());
  BOOST_CHECK_EQUAL("", fURI("fos://pig@127.0.0.1/a/b").password());
  BOOST_CHECK_EQUAL("pass", fURI("fos://pig:pass@127.0.0.1/a/b").password());
  BOOST_CHECK_EQUAL("x", fURI("//x@/a/b/c/d/e").user());
};

BOOST_AUTO_TEST_CASE(test_uri_host) {
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//127.0.0.1/a/b").host());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//fhat@127.0.0.1/a/b").host());
  BOOST_CHECK_EQUAL("", fURI("fhat@127.0.0.1/a/b").host());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("xxx://fhat:pig@127.0.0.1/a/b").host());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//fhat:pig@127.0.0.1/a/b").host());
  BOOST_CHECK_EQUAL("", fURI("/a/b/c").host());
  BOOST_CHECK_EQUAL("", fURI("fhat@/a/b/c").host());
  /////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a"), fURI("/a").host("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/a/b/c"), fURI("/a/b/c").host("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("//127.0.0.1/"), fURI("/").host("127.0.0.1"));
};

BOOST_AUTO_TEST_CASE(test_uri_authority) {
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//127.0.0.1").authority().c_str());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//127.0.0.1/a").authority().c_str());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//127.0.0.1/a/b").authority().c_str());
  BOOST_CHECK_EQUAL("127.0.0.1", fURI("//127.0.0.1/a/b/c").authority().c_str());
  BOOST_CHECK_EQUAL(fURI("//127.0.0.1/a/b/c").authority().c_str(), fURI("//127.0.0.1/d/e").authority().c_str());
  BOOST_CHECK_EQUAL("fat@127.0.0.1", fURI("//fat@127.0.0.1/a/b/c").authority().c_str());
  BOOST_CHECK_EQUAL("fat:pig@127.0.0.1", fURI("//fat:pig@127.0.0.1/a/b/c").authority().c_str());
  BOOST_CHECK_EQUAL("", fURI("/a").authority().c_str());
  BOOST_CHECK_EQUAL("", fURI("x@/a/b/c/d/e").authority().c_str());
  BOOST_CHECK_EQUAL("x@", fURI("//x@/a/b/c/d/e").authority().c_str());
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
  BOOST_ASSERT(fURI("/a/b/c").authority().empty());
};

BOOST_AUTO_TEST_CASE(test_uri_path) {
  BOOST_CHECK_EQUAL("", fURI("").path().c_str());
  BOOST_CHECK_EQUAL("", fURI("//127.0.0.1").path().c_str());
  BOOST_CHECK_EQUAL("/a", fURI("//127.0.0.1/a").path().c_str());
  BOOST_CHECK_EQUAL("/a/b/c", fURI("//127.0.0.1/a/b/c").path().c_str());
  BOOST_CHECK_EQUAL("/a/b/c", fURI("//fhat@127.0.0.1/a/b/c").path().c_str());
  BOOST_CHECK_EQUAL("/a/b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path(0, 2).c_str());
  BOOST_CHECK_EQUAL("b", fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path(1));
  BOOST_CHECK_EQUAL("c/d", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").path(2, 4).c_str());
  BOOST_CHECK_EQUAL("e", fURI("fos://fhat@127.0.0.1/a/b/c/d/e").path(4, 5).c_str());
  BOOST_CHECK_EQUAL("e/", fURI("fos://fhat@127.0.0.1/a/b/c/d/e/").path(4, 5).c_str());
  BOOST_CHECK_EQUAL("", fURI("//fhat@127.0.0.1/a/b/c/d/e").path(5, 6).c_str());
  BOOST_CHECK_EQUAL("", fURI("//fhat@127.0.0.1/a/b/c/d/e").path(6));
  BOOST_CHECK_EQUAL("/a/b/c/d/e", fURI("/a/b/c/d/e").path().c_str());
  BOOST_CHECK_EQUAL("/a/b/c/d/e", fURI("//x@/a/b/c/d/e").path().c_str());
  BOOST_CHECK_EQUAL("c/d", fURI("/a/b/c/d/e").path(2, 4).c_str());
  BOOST_CHECK_EQUAL("c/d", fURI("//x@/a/b/c/d/e").path(2, 4).c_str());
  //
  BOOST_CHECK_EQUAL(0, fURI("").path_length());
  BOOST_CHECK_EQUAL(0, fURI("foi://fhat@127.0.0.1").path_length());
  BOOST_CHECK_EQUAL(1, fURI("fos://fhat@127.0.0.1/a").path_length());
  BOOST_CHECK_EQUAL(5, fURI("furi://fhat@127.0.0.1/a/b/c/d/e").path_length());
  BOOST_CHECK_EQUAL(5, fURI("/a/b/c/d/e").path_length());
  BOOST_CHECK_EQUAL(5, fURI("//x@/a/b/c/d/e").path_length());
  BOOST_CHECK_EQUAL(5, fURI("a/b/c/d/e").path_length());
  BOOST_CHECK_EQUAL(4, fURI("//x@a/b/c/d/e").path_length());
  //
  BOOST_CHECK_EQUAL("a", fURI("//127.0.0.1/a/b/c").path(0));
  BOOST_CHECK_EQUAL("b", fURI("//127.0.0.1/a/b/c").path(1));
  BOOST_CHECK_EQUAL("c", fURI("//127.0.0.1/a/b/c").path(2));
  BOOST_CHECK_EQUAL("", fURI("//127.0.0.1/a/b/c").path(3));
  BOOST_CHECK_EQUAL("a", fURI("//127.0.0.1///a//b////c").path(2));
  BOOST_CHECK_EQUAL("", fURI("//127.0.0.1///a//b////c").path(3));
  BOOST_CHECK_EQUAL("b", fURI("//127.0.0.1///a//b////c").path(4));
  BOOST_CHECK_EQUAL("c", fURI("//127.0.0.1///a//b////c").path(8));
}

BOOST_AUTO_TEST_CASE(test_uri_query) {
  BOOST_ASSERT(0 == strlen(fURI("127.0.0.1").query()));
  BOOST_CHECK_EQUAL("testing", fURI("127.0.0.1/a/b?testing").query());
  BOOST_CHECK_EQUAL("testing=123", fURI("127.0.0.1?testing=123").query());
  BOOST_CHECK_EQUAL("a=1;b=2", fURI("fhat@127.0.0.1?a=1;b=2").query());
  BOOST_CHECK_EQUAL("a;b;c", fURI("/a/b/c?a;b;c").query());
  BOOST_CHECK_EQUAL("query", fURI("127.0.0.1/a?query").query());
  BOOST_CHECK_EQUAL("", fURI("127.0.0.1/a?").query());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a?query").query(""));
  BOOST_CHECK_EQUAL("127.0.0.1/a?testing", fURI("127.0.0.1/a/b?testing").retract().toString().c_str());
  ////////////////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a?a=1"), fURI("127.0.0.1/a").query("a=1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1/a?a=1;b=2;c=3"), fURI("fhat@127.0.0.1/a").query("a=1;b=2;c=3"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a?a=1;b=2;c=3"), fURI("/a").query("a=1;b=2;c=3"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("?a,b,c"), fURI("").query("a,b,c"));
};

BOOST_AUTO_TEST_CASE(test_uri_empty) {
  BOOST_ASSERT(fURI("").empty());
  BOOST_ASSERT(!fURI("fos:").empty());
  BOOST_ASSERT(!fURI("a/b/c").empty());
  BOOST_ASSERT(!fURI("http://a.com:34/b/c#det").empty());
};

BOOST_AUTO_TEST_CASE(test_uri_extend) {
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

BOOST_AUTO_TEST_CASE(test_uri_resolve) {
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

BOOST_AUTO_TEST_CASE(test_uri_match) {
  //// TRUE
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1"), fURI("+"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1"), fURI("#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("fhat@127.0.0.1/a"), fURI("fhat@127.0.0.1/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/abc.org"), fURI("127.0.0.1/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/#/b"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/+/b"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/+/+"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/a/+/c"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/a/+/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c/d"), fURI("127.0.0.1/a/+/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/#/x/v"));
  // TODO: ?? TEST_ASSERT_TRUE(fURI("127.0.0.1"),fURI("127.0.0.1/#"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("fos://127.0.0.1/a"), fURI("fos://127.0.0.1/a"));
  FOS_TEST_ASSERT_MATCH_FURI(fURI("fos://127.0.0.1:21/a/b"), fURI("fos://127.0.0.1:21/+/b"));
  //// FALSE
  FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1"), fURI("127.0.0.2"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.2/?/b"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1"), fURI("127.0.0.1/+"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/a/b/c"), fURI("127.0.0.1/+/+"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(fURI("127.0.0.1/abc"), fURI("127.0.0.1/abc/#"));
};

BOOST_AUTO_TEST_CASE(test_fhat_idioms) {
  fURI nat("/int/nat");
  BOOST_CHECK_EQUAL("nat", nat.name());
  BOOST_CHECK_EQUAL("/int/nat", nat.toString().c_str());
  BOOST_CHECK_EQUAL("int", nat.path(0));
  BOOST_CHECK_EQUAL("nat", nat.path(1));
  BOOST_CHECK_EQUAL("", nat.path(3));
  BOOST_CHECK_EQUAL(2, nat.path_length());
  BOOST_CHECK_EQUAL("/", nat.path(0, 0).c_str());
  BOOST_CHECK_EQUAL("/int", nat.path(0, 1).c_str());
  BOOST_CHECK_EQUAL("", nat.path(2, 1).c_str());
  BOOST_CHECK_EQUAL("", nat.path(2, 3).c_str());
  BOOST_CHECK_EQUAL("/int/nat", nat.path(0, 2).c_str());
  BOOST_CHECK_EQUAL("nat", nat.path(1, 2).c_str());
  BOOST_CHECK_EQUAL("", nat.path(2, 2).c_str());
  FOS_TEST_ASSERT_EQUAL_FURI(ID("/int/nat"), ID("/int/").resolve(ID("nat")));
  FOS_TEST_ASSERT_EQUAL_FURI(ID("/int/"), ID("/int/").resolve(ID("")));
}

BOOST_AUTO_TEST_CASE(test_pattern_pattern_matching) {
  FOS_TEST_ASSERT_MATCH_FURI(*p_p("/fs/#"), *p_p("#"));
  FOS_TEST_ASSERT_MATCH_FURI(*p_p("/fs/mount/#"), *p_p("/fs/#"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(*p_p("/#"), *p_p("/fs/#"));
  FOS_TEST_ASSERT_MATCH_FURI(*p_p("/fs/#"), *p_p("/fs/#"));
  FOS_TEST_ASSERT_MATCH_FURI(*p_p("/fs/+/abc"), *p_p("/fs/#"));
  FOS_TEST_ASSERT_NOT_MATCH_FURI(*p_p("/fs/#"), *p_p("/fs/+/abc"));
}

BOOST_AUTO_TEST_CASE(test_composite_mutations) {
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


BOOST_AUTO_TEST_SUITE_END()

#endif
