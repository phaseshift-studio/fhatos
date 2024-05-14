#ifndef fhatos_test_message_hpp
#define fhatos_test_message_hpp

#include <test_fhatos.hpp>
//
#include <structure/furi.hpp>
#include <process/router/message.hpp>
#include <language/serializer.hpp>

namespace fhatos {

void test_bool() {
  for (int i = 0; i < 1000; i++) {
    ///// BOOL
    if (i % 100 == 0) {
      FOS_TEST_MESSAGE("Testing bool conversion: %i", i);
    }
    Message m{.source = ID("a"),
              .target = ID("b"),
              .payload = SerialObj<>::fromBoolean((i % 2) == 0),
              .retain = (i % 2) != 0};
    TEST_ASSERT_EQUAL(BOOL, m.payload.type);
    if (i % 2 == 0) {
      TEST_ASSERT_TRUE(m.payload.toBool().value());
      TEST_ASSERT_EQUAL(m.payload.data[0], 'T');
    } else {
      TEST_ASSERT_FALSE(m.payload.toBool().value());
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
              .payload = SerialObj<>::fromInteger(i),
              .retain = (i % 2) == 0};
    TEST_ASSERT_EQUAL(INT, m.payload.type);
    TEST_ASSERT_EQUAL(i, m.payload.toInt().value());
    TEST_ASSERT_EQUAL(m.payload.length, strlen((const char *)m.payload.data));
    TEST_ASSERT_EQUAL((i % 2) == 0, m.retain);
  }
}

void test_float() {
  ///// FLOAT
  int counter = 0;
  for (float i = -200000.123f; i < 200000.123f; i = i + 5.196f) {
    if (counter++ % 5000 == 0) {
      FOS_TEST_MESSAGE("Testing float conversion: %.4f", i);
    }
    Message m{.source = ID("a"),
              .target = ID("b"),
              .payload = SerialObj<>::fromFloat(i),
              .retain = i > 0.0f};
    TEST_ASSERT_EQUAL(REAL, m.payload.type);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, i, m.payload.toReal().value());
    TEST_ASSERT_EQUAL(m.payload.length, strlen((const char *)m.payload.data));
    TEST_ASSERT_EQUAL(i > 0.0f, m.retain);
  }
}

void test_str() {
  ///// STRING
  Message m{.source = ID("a"),
            .target = ID("b"),
            .payload = SerialObj<>::fromString("fhatty"),
            .retain = false};
  TEST_ASSERT_EQUAL_STRING("fhatty", m.payload.toString().c_str());
  TEST_ASSERT_EQUAL_STRING("fhatty", (const char *)m.payload.data);
  TEST_ASSERT_EQUAL(6, m.payload.length);
  TEST_ASSERT_EQUAL(STR, m.payload.type);
  TEST_ASSERT_FALSE(m.retain);
}

void test_interpret() {
  TEST_ASSERT_TRUE(
     SerialObj<>::fromString("fhat").equals(SerialObj<>::interpret("\"fhat\"")));
  TEST_ASSERT_TRUE(SerialObj<>::fromBoolean(true).equals(SerialObj<>::interpret("true")));
  TEST_ASSERT_TRUE(SerialObj<>::fromInteger(62).equals(SerialObj<>::interpret("62")));
  //TEST_ASSERT_TRUE(
    // SerialObj<>::fromFloat(12.32f).equals(SerialObj<>::interpret("12.32f")));
  //////////////////////////////////////////////////////////////////
  TEST_ASSERT_EQUAL_STRING(SerialObj<>::fromString("fhat").toStr().value().c_str(),
                           SerialObj<>::interpret("\"fhat\"").toStr().value().c_str());
  TEST_ASSERT_EQUAL(SerialObj<>::fromBoolean(true).toBool().value(),
                    SerialObj<>::interpret("true").toBool().value());
  TEST_ASSERT_FLOAT_WITHIN(0.1f, SerialObj<>::fromFloat(12.32f).toReal().value(),
                          SerialObj<>::interpret("12.32f").toReal().value());
  TEST_ASSERT_EQUAL(SerialObj<>::fromInteger(62).toInt().value(),
                    SerialObj<>::interpret("62").toInt().value());
  //////////////////////////////////////////////////////////////////
  TEST_ASSERT_EQUAL_STRING("fhat",
                           SerialObj<>::interpret("\"fhat\"").toStr().value().c_str());
  TEST_ASSERT_TRUE(SerialObj<>::interpret("true").toBool().value());
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 12.32f,
                           SerialObj<>::interpret("12.32f").toReal().value());
  TEST_ASSERT_EQUAL(62, SerialObj<>::interpret("62").toInt().value());
  //////////////////////////////////////////////////////////////////
  TEST_ASSERT_EQUAL_STRING("fhat",
                         SerialObj<>::interpret("\"fhat\"").toStr().value().c_str());
  TEST_ASSERT_EQUAL_STRING("true",
                          SerialObj<>::interpret("true").toStr().value().c_str());
  TEST_ASSERT_EQUAL_STRING("10.320000",
                           SerialObj<>::interpret("10.32f").toReal().toString().c_str()); // TODO: fix
  TEST_ASSERT_EQUAL_STRING("24", SerialObj<>::interpret("24").toStr().value().c_str());
}

FOS_RUN_TESTS(                    //
    FOS_RUN_TEST(test_bool);      //
    FOS_RUN_TEST(test_int);       //
    FOS_RUN_TEST(test_float);     //
    FOS_RUN_TEST(test_str);       //
    FOS_RUN_TEST(test_interpret); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif
