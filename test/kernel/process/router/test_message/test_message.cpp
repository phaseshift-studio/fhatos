#ifndef fhatos_kernel__test_message_hpp
#define fhatos_kernel__test_message_hpp

#include <test_fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/process/router/message.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_str() {
  ///// STRING
  Message message1{.source = ID("a"),
                   .target = ID("b"),
                   .payload = Payload::fromString("fhatty"),
                   .retain = false};
  TEST_ASSERT_EQUAL_STRING("fhatty", message1.payload.toString().c_str());
  TEST_ASSERT_EQUAL_STRING("fhatty", (const char *)message1.payload.data);
  TEST_ASSERT_EQUAL(6, message1.payload.length);
  TEST_ASSERT_EQUAL(STR, message1.payload.type);
  TEST_ASSERT_FALSE(message1.retain);
}

void test_int() {
  ///// INT
  for (int i = -200000; i < 200000; i = i + 5) {
    if (i % 50000 == 0) {
      FOS_TEST_MESSAGE("Testing int conversion: %i", i);
    }
    Message message2{.source = ID("a"),
                     .target = ID("b"),
                     .payload = Payload::fromInt(i),
                     .retain = true};
    TEST_ASSERT_EQUAL(i, message2.payload.toInt());
    TEST_ASSERT_EQUAL(INT, message2.payload.type);
    TEST_ASSERT_EQUAL(message2.payload.length,
                      strlen((const char *)message2.payload.data));
    TEST_ASSERT_TRUE(message2.retain);
  }
}

void test_bool() {
  for (int i = 0; i < 1000; i++) {
    ///// BOOL
    if (i % 100 == 0) {
      FOS_TEST_MESSAGE("Testing bool conversion: %i", i);
    }
    Message message3{.source = ID("a"),
                     .target = ID("b"),
                     .payload = Payload::fromBool((i % 2) == 0),
                     .retain = (i % 2) != 0};
    TEST_ASSERT_EQUAL(BOOL, message3.payload.type);
    if (i % 2 == 0) {
      TEST_ASSERT_TRUE(message3.payload.toBool());
    } else {
      TEST_ASSERT_FALSE(message3.payload.toBool());
    }
    TEST_ASSERT_EQUAL(1, message3.payload.length);
    TEST_ASSERT_EQUAL((i % 2) != 0, message3.retain);
  }
}

FOS_RUN_TESTS(               //
    FOS_RUN_TEST(test_str);  //
    FOS_RUN_TEST(test_int);  //
    FOS_RUN_TEST(test_bool); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
