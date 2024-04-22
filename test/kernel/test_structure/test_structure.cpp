#ifndef fhatos_kernel__test_structure_hpp
#define fhatos_kernel__test_structure_hpp

#include <test_fhatos.hpp>
//
#include <kernel/structure/structure.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_furi_equals() {
  /// TRUE
  TEST_ASSERT_TRUE(fURI("").equals(fURI("")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1").equals(fURI("127.0.0.1")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b/")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a/b//")));
  TEST_ASSERT_TRUE(fURI("127.0.0.1/a/b").equals(fURI("127.0.0.1/a///b")));
  // MACRO EQUAL FURI
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1/a///b"));
  /// FALSE
  TEST_ASSERT_FALSE(fURI("127.0.0.1").equals(fURI("127.1.1.2")));
  TEST_ASSERT_FALSE(fURI("127.0.0.1/a").equals(fURI("127.0.0.1/b")));
  TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a/b"));
}

void test_furi_length() {
  // empty()
  TEST_ASSERT_TRUE(fURI("").empty());
  TEST_ASSERT_TRUE(fURI("/").empty());
  TEST_ASSERT_FALSE(fURI("127.0.0.1").empty());
  /// length()
  TEST_ASSERT_EQUAL_INT(fURI("").length(), 0);
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
}

void test_furi_extend() {
  /// TRUE
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1").extend("a"));
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1/a").extend(""));
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                         fURI("127.0.0.1").extend("a").extend("b"));
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"),
                         fURI("127.0.0.1").extend("a/b"));

  //// FALSE
  TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("127.0.0.1").extend("a").extend("b"));
  TEST_ASSERT_NOT_EQUAL_FURI(fURI("127.0.0.1/a"),
                             fURI("127.0.0.1").extend("b"));
}

void test_furi_slash_operator() {
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a"), fURI("127.0.0.1") / "a");
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b"), fURI("127.0.0.1") / "a" / "b");
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"),
                         fURI("127.0.0.1") / "a" / "b/c");
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"),
                         fURI("127.0.0.1") / "a" / "b/c" / "" / "/");
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
  TEST_ASSERT_EXCEPTION(ID("127.0.0.1/#"));
  TEST_ASSERT_EQUAL_FURI(fURI("127.0.0.1/a/b/c"), ID("127.0.0.1/a/b/c"));
}

RUN_TESTS(                              //
    RUN_TEST(test_furi_equals);         //
    RUN_TEST(test_furi_length);         //
    RUN_TEST(test_furi_user_password);  //
    RUN_TEST(test_furi_host);           //
    RUN_TEST(test_furi_segment);        //
    RUN_TEST(test_furi_authority);      //
    RUN_TEST(test_furi_extend);         //
    RUN_TEST(test_furi_slash_operator); //
    RUN_TEST(test_furi_match);          //
    RUN_TEST(test_id_construction);     //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif