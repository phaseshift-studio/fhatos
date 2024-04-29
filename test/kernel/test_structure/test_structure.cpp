#ifndef fhatos_kernel__test_structure_hpp
#define fhatos_kernel__test_structure_hpp

#include <test_fhatos.hpp>
//
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_furi_memory_leaks() {
  int sketchMemory = -1;
  int heapMemory = -1;
  for (int i = 0; i < 20000; i++) {
    fURI a = fURI("127.0.0.1");
    fURI b = fURI(a);
    fURI c = fURI(b.toString());
    fURI d = fURI(c.segment(0));
    fURI e = fURI(d.segment(0)).extend("");
    fURI f = fURI(e.toString());
    TEST_ASSERT_TRUE(a.equals(a));
    TEST_ASSERT_TRUE(a.equals(b));
    TEST_ASSERT_TRUE(a.equals(c));
    TEST_ASSERT_TRUE(a.equals(d));
    TEST_ASSERT_TRUE(a.equals(e));
    TEST_ASSERT_TRUE(a.equals(f));
    if (sketchMemory != -1) {
      TEST_ASSERT_EQUAL_INT32(sketchMemory, ESP.getFreeSketchSpace());
      TEST_ASSERT_EQUAL_INT32(heapMemory, ESP.getFreeHeap());
    }
    sketchMemory = ESP.getFreeSketchSpace();
    heapMemory = ESP.getFreeHeap();
    if (i % 1000 == 0) {
      FOS_TEST_MESSAGE("fURI count: %i\t[free sketch:%i][free heap:%i]", i,
                       sketchMemory, heapMemory);
    }
  }
  FOS_TEST_MESSAGE("FINAL [free sketch:%i][free heap:%i]",
                   ESP.getFreeSketchSpace(), ESP.getFreeHeap());
}

void test_furi_equals() {
  /// TRUE
  TEST_ASSERT_TRUE(fURI("").equals(fURI("")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1").equals(fURI("127.0.0.1")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b")));
  TEST_ASSERT_TRUE(
      fURI("fhat@127.0.0.1/a/b").equals(fURI("fhat@127.0.0.1/a/b")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b/")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b//")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a///b")));
  // MACRO EQUAL FURI
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/a///b"));
  /// FALSE
  TEST_ASSERT_FALSE(fURI("127.0.0.1").equals(fURI("127.1.1.2")));
  TEST_ASSERT_FALSE(fURI("127.0.0.1/a").equals(fURI("127.0.0.1/b")));
  FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a/b"));
  FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("fhat@127.0.0.1/a"),
                                 fURI("pig@127.0.0.1/a"));
}

void test_furi_length() {
  // empty()
  TEST_ASSERT_TRUE(fURI("").empty());
  TEST_ASSERT_TRUE(fURI("/").empty());
  TEST_ASSERT_FALSE(fURI("127.0.0.1").empty());
  /// length()
  TEST_ASSERT_EQUAL_INT(fURI("").length(), 0);
  TEST_ASSERT_EQUAL_INT(fURI("/").length(), 0);
  TEST_ASSERT_EQUAL_INT(fURI("127.0.0.1").length(), 1);
  TEST_ASSERT_EQUAL_INT(fURI("127.0.0.1/a").length(), 2);
  TEST_ASSERT_EQUAL_INT(fURI("127.0.0.1/a/b/c").length(), 4);
  TEST_ASSERT_EQUAL_INT(fURI("127.0.0.1/a///b///c").length(), 4);
}

void test_furi_user_password() {
  TEST_ASSERT_EQUAL_STRING(
      "fhat", fURI("fhat@127.0.0.1/a/b").user_password()->first.c_str());
  TEST_ASSERT_EQUAL_STRING(
      "", fURI("fhat@127.0.0.1/a/b").user_password()->second.c_str());
  TEST_ASSERT_EQUAL_STRING(
      "fhat", fURI("fhat:pig@127.0.0.1/a/b").user_password()->first.c_str());
  TEST_ASSERT_EQUAL_STRING(
      "pig", fURI("fhat:pig@127.0.0.1/a/b").user_password()->second.c_str());
  TEST_ASSERT_EQUAL_STRING(
      "", fURI(":pig@127.0.0.1/a/b").user_password()->first.c_str());
  TEST_ASSERT_EQUAL_STRING(
      "pig", fURI(":pig@127.0.0.1/a/b").user_password()->second.c_str());

  ///
  TEST_ASSERT_FALSE(fURI("127.0.0.1/a/b").user_password().has_value());
}

void test_furi_host() {
  TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("127.0.0.1/a/b").host().c_str());
  TEST_ASSERT_EQUAL_STRING("127.0.0.1",
                           fURI("fhat@127.0.0.1/a/b").host().c_str());
  TEST_ASSERT_EQUAL_STRING("127.0.0.1",
                           fURI("fhat:pig@127.0.0.1/a/b").host().c_str());
  TEST_ASSERT_EQUAL_STRING("", fURI("/a/b/c").host().c_str());
  TEST_ASSERT_EQUAL_STRING("", fURI("fhat@/a/b/c").host().c_str());
  /////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("/a").host("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"),
                             fURI("/a/b/c").host("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"), fURI("/").host("127.0.0.1"));
}

void test_furi_resolve() {
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1/a"),
                             fURI("fhat@/a").resolve(fURI("127.0.0.1/a/b/c")));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("/a").resolve(fURI("127.0.0.1")));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                             fURI("/b").resolve(fURI("127.0.0.1/a")));
}

void test_furi_path() {
  TEST_ASSERT_EQUAL_STRING("", fURI("").path().c_str());
  TEST_ASSERT_EQUAL_STRING("", fURI("127.0.0.1").path().c_str());
  TEST_ASSERT_EQUAL_STRING("a", fURI("127.0.0.1/a").path().c_str());
  TEST_ASSERT_EQUAL_STRING("a/b/c", fURI("127.0.0.1/a/b/c").path().c_str());
  TEST_ASSERT_EQUAL_STRING("a/b/c",
                           fURI("fhat@127.0.0.1/a/b/c").path().c_str());
}

void test_furi_segment() {
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1/a/b/c").segment(0).c_str(),
                           "127.0.0.1");
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1/a/b/c").segment(1).c_str(), "a");
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1/a/b/c").segment(2).c_str(), "b");
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1/a/b/c").segment(3).c_str(), "c");
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1///a//b////c").segment(3).c_str(),
                           "c");
}

void test_furi_authority() {
  TEST_ASSERT_EQUAL_STRING("127.0.0.1", fURI("127.0.0.1").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("127.0.0.1",
                           fURI("127.0.0.1/a").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("127.0.0.1",
                           fURI("127.0.0.1/a/b").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("127.0.0.1",
                           fURI("127.0.0.1/a/b/c").authority().c_str());
  TEST_ASSERT_EQUAL_STRING(fURI("127.0.0.1/a/b/c").authority().c_str(),
                           fURI("127.0.0.1/d/e").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("fat@127.0.0.1",
                           fURI("fat@127.0.0.1/a/b/c").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("fat:pig@127.0.0.1",
                           fURI("fat:pig@127.0.0.1/a/b/c").authority().c_str());
  TEST_ASSERT_EQUAL_STRING("", fURI("/a").authority().c_str());
  //////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("1.1.1.1/a").authority("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"),
                             fURI("1.1.1.1").authority("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("1.1.1.1/a").authority("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("/a").authority("127.0.0.1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1"),
                             fURI("").authority("127.0.0.1"));
  TEST_ASSERT_TRUE(fURI("/a/b/c").authority().isEmpty());
}

void test_furi_retract() {
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("127.0.0.1/a/b").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1/a"),
                             fURI("fhat@127.0.0.1/a/b").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1"),
                             fURI("fhat@127.0.0.1/a/b").retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(
      fURI(""), fURI("fhat@127.0.0.1/a/b").retract().retract().retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("fhat@127.0.0.1/a/b")
                                           .retract()
                                           .retract()
                                           .retract()
                                           .retract()
                                           .retract());
  FOS_TEST_ASSERT_EQUAL_FURI(fURI(""), fURI("").retract());
  FOS_TEST_ASSERT_EQUAL_FURI(
      fURI("fhat@127.0.0.1/a"),
      fURI("fhat@127.0.0.1/a/b").retract().retract().extend("a"));
}

void test_furi_query() {
  TEST_ASSERT_TRUE(fURI("127.0.0.1").query().isEmpty());
  TEST_ASSERT_EQUAL_STRING("testing",
                           fURI("127.0.0.1/a/b?testing").query().c_str());
  TEST_ASSERT_EQUAL_STRING("testing=123",
                           fURI("127.0.0.1?testing=123").query().c_str());
  TEST_ASSERT_EQUAL_STRING("a=1;b=2",
                           fURI("fhat@127.0.0.1?a=1;b=2").query().c_str());
  TEST_ASSERT_EQUAL_STRING("a;b;c", fURI("/a/b/c?a;b;c").query().c_str());
  ////////////////
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a?a=1"),
                             fURI("127.0.0.1/a").query("a=1"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("fhat@127.0.0.1/a?a=1;b=2;c=3"),
                             fURI("fhat@127.0.0.1/a").query("a=1;b=2;c=3"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("/a?a=1;b=2;c=3"),
                             fURI("/a").query("a=1;b=2;c=3"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("?a,b,c"), fURI("").query("a,b,c"));
}

void test_furi_extend() {
  /// TRUE
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("127.0.0.1").extend("a"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("127.0.0.1/a").extend(""));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                             fURI("127.0.0.1").extend("a").extend("b"));
  FOS_TEST_ASSERT_EQUAL_FURI(
      fURI("fhat:pig@127.0.0.1/a/b"),
      fURI("fhat:pig@127.0.0.1").extend("a").extend("b"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                             fURI("127.0.0.1").extend("a").extend("b"));

  //// FALSE
  FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"),
                                 fURI("127.0.0.1").extend("a").extend("b"));
  FOS_TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"),
                                 fURI("127.0.0.1").extend("b"));
}

void test_furi_slash_operator() {
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1") / "a");
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                             fURI("127.0.0.1") / "a" / "b");
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"),
                             fURI("127.0.0.1") / "a" / "b" / "c");
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"),
                             fURI("127.0.0.1") / "a" / "b" / "c" / "" / "");
}

void test_furi_match() {
  //// TRUE
  TEST_ASSERT_TRUE(ID("127.0.0.1/a").matches(fURI("127.0.0.1/a")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a").matches(ID("127.0.0.1/a")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a").matches(Pattern("127.0.0.1/a")));
  TEST_ASSERT_TRUE(ID("127.0.0.1").matches(Pattern("+")));
  TEST_ASSERT_TRUE(ID("127.0.0.1").matches(Pattern("#")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a").matches(Pattern("127.0.0.1/#")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b").matches(Pattern("127.0.0.1/#/b")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b").matches(Pattern("127.0.0.1/+/b")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b").matches(Pattern("127.0.0.1/+/+")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b/c").matches(Pattern("127.0.0.1/a/+/c")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b/c").matches(Pattern("127.0.0.1/a/+/#")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b/c/d").matches(Pattern("127.0.0.1/a/+/#")));
  TEST_ASSERT_TRUE(ID("127.0.0.1/a/b/c").matches(Pattern("127.0.0.1/#/x/v")));
  // TODO: ?? TEST_ASSERT_TRUE(ID("127.0.0.1").matches(fURI("127.0.0.1/#")));
  //// FALSE
  TEST_ASSERT_FALSE(ID("127.0.0.1").matches(fURI("127.0.0.2")));
  TEST_ASSERT_FALSE(ID("127.0.0.1/a/b").matches(fURI("127.0.0.2/?/b")));
  TEST_ASSERT_FALSE(ID("127.0.0.1").matches(fURI("127.0.0.1/+")));
  TEST_ASSERT_FALSE(ID("127.0.0.1/a/b/c").matches(fURI("127.0.0.1/+/+")));
}

void test_id_construction() {
  // TODO: Exception in constructor causes memory leak:
  // FOS_TEST_ASSERT_EXCEPTION(ID("127.0.0.1/#"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"), ID("127.0.0.1/a/b/c"));
  FOS_TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/#"),
                             Pattern("127.0.0.1/a/b/#"));
}

FOS_RUN_TESTS(                              //
    FOS_RUN_TEST(test_furi_memory_leaks);   //
    FOS_RUN_TEST(test_furi_equals);         //
    FOS_RUN_TEST(test_furi_length);         //
    FOS_RUN_TEST(test_furi_user_password);  //
    FOS_RUN_TEST(test_furi_host);           //
    FOS_RUN_TEST(test_furi_path);           //
    FOS_RUN_TEST(test_furi_resolve);        //
    FOS_RUN_TEST(test_furi_segment);        //
    FOS_RUN_TEST(test_furi_authority);      //
    FOS_RUN_TEST(test_furi_query);          //
    FOS_RUN_TEST(test_furi_retract);        //
    FOS_RUN_TEST(test_furi_extend);         //
    FOS_RUN_TEST(test_furi_slash_operator); //
    FOS_RUN_TEST(test_furi_match);          //
    FOS_RUN_TEST(test_id_construction);     //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif