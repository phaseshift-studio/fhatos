#ifndef fhatos_kernel__test_message_hpp
#define fhatos_kernel__test_message_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/process/router/message.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_bool() {
  for (int i = 0; i < 1000; i++) {
    ///// BOOL
    if (i % 100 == 0) {
      FOS_TEST_MESSAGE("Testing bool conversion: %i", i);
    }
    Message m{.source = ID("a"),
              .target = ID("b"),
              .payload = Payload::fromBool((i % 2) == 0),
              .retain = (i % 2) != 0};
    TEST_ASSERT_EQUAL(BOOL, m.payload.type);
    if (i % 2 == 0) {
      TEST_ASSERT_TRUE(m.payload.toBool());
      TEST_ASSERT_EQUAL(m.payload.data[0], 'T');
    } else {
      TEST_ASSERT_FALSE(m.payload.toBool());
      TEST_ASSERT_EQUAL(m.payload.data[0], 'F');
    }
    TEST_ASSERT_EQUAL(1, m.payload.length);
    TEST_ASSERT_EQUAL((i % 2) != 0, m.retain);
  }
}

void test_int() {
  ///// INT
  for (int i = -200000; i < 200000; i = i + 5) {
    if (i % 50000 == 0) {
      FOS_TEST_MESSAGE("Testing int conversion: %i", i);
    }
    Message m{.source = ID("a"),
              .target = ID("b"),
              .payload = Payload::fromInt(i),
              .retain = (i % 2) == 0};
    TEST_ASSERT_EQUAL(INT, m.payload.type);
    TEST_ASSERT_EQUAL(i, m.payload.toInt());
    TEST_ASSERT_EQUAL(m.payload.length, strlen((const char *)m.payload.data));
    TEST_ASSERT_EQUAL((i % 2) == 0, m.retain);
  }
}

void test_float() {
  ///// FLOAT
  for (float i = -200000.123f; i < 200000.123f; i = i + 5.0f) {

    if (((int)i) % 50000 == 0) {
      FOS_TEST_MESSAGE("Testing float conversion: %.4f", i);
    }
    Message m{.source = ID("a"),
              .target = ID("b"),
              .payload = Payload::fromFloat(i),
              .retain = i > 0.0f};
    TEST_ASSERT_EQUAL(REAL, m.payload.type);
    TEST_ASSERT_EQUAL(i, m.payload.toFloat());
    TEST_ASSERT_EQUAL(m.payload.length, strlen((const char *)m.payload.data));
    TEST_ASSERT_EQUAL(i > 0.0f, m.retain);
  }
}

void test_str() {
  ///// STRING
  Message m{.source = ID("a"),
            .target = ID("b"),
            .payload = Payload::fromString("fhatty"),
            .retain = false};
  TEST_ASSERT_EQUAL_STRING("fhatty", m.payload.toString().c_str());
  TEST_ASSERT_EQUAL_STRING("fhatty", (const char *)m.payload.data);
  TEST_ASSERT_EQUAL(6, m.payload.length);
  TEST_ASSERT_EQUAL(STR, m.payload.type);
  TEST_ASSERT_FALSE(m.retain);
}

FOS_RUN_TESTS(                //
    FOS_RUN_TEST(test_bool);  //
    FOS_RUN_TEST(test_int);   //
    FOS_RUN_TEST(test_float); //
    FOS_RUN_TEST(test_str);   //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
