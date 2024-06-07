#ifndef fhatos_test_binary_obj_hpp
#define fhatos_test_binary_obj_hpp

#include <test_fhatos.hpp>
//
#include <language/obj.hpp>
#include <language/binary_obj.hpp>

namespace fhatos {

 ////////////////////////////// BOOL ///////////////////////////////////
void test_bool() {
  for (int i = 0; i < 1000; i++) {
    if (i % 100 == 0) {
      FOS_TEST_MESSAGE("Testing bool conversion: %i", i);
    }
bool flip = i % 2 == 0;
    BinaryObj<> m =  flip ? BinaryObj<>(flip) : BinaryObj<>::interpret(flip ? "true" : "false");
    TEST_ASSERT_EQUAL(OType::BOOL, m.type());
	if (flip) {
      TEST_ASSERT_TRUE(m.toBool().value());
      TEST_ASSERT_EQUAL(m.data()[0], 'T');
    } else {
      TEST_ASSERT_FALSE(m.toBool().value());
      TEST_ASSERT_EQUAL(m.data()[0], 'F');
    }
    TEST_ASSERT_EQUAL(1, m.length());
	TEST_ASSERT_EQUAL_STRING(m.toString().c_str(), m.toStr().toString().c_str());
	TEST_ASSERT_TRUE(m.equals(BinaryObj<>::interpret(flip ? "true" : "false")));
	//TEST_ASSERT_TRUE(m.payload.equals(m.payload.toInt().toStr().toBool()));
  }
}
 ////////////////////////////// INT ///////////////////////////////////
void test_int() {
  for (int i = -200000; i < 200000; i = i + 5) {
    if (i % 50000 == 0) {
      FOS_TEST_MESSAGE("Testing int conversion: %i", i);
    }
	bool flip = (i % 2) == 0;
 BinaryObj<> m = flip ? BinaryObj<>(i) : BinaryObj<>::interpret(std::to_string(i));
    TEST_ASSERT_EQUAL(OType::INT, m.type());
    TEST_ASSERT_EQUAL(i, m.toInt().value());
    TEST_ASSERT_EQUAL(m.length(), strlen((const char *)m.data()));
	TEST_ASSERT_EQUAL_STRING(m.toString().c_str(), m.toStr().toString().c_str());
	TEST_ASSERT_TRUE(m.equals(BinaryObj<>::interpret(std::to_string(i))));
	//TEST_ASSERT_TRUE(m.payload.equals(m.payload.toReal().toStr().toInt()));
  }
}

 ////////////////////////////// REAL ///////////////////////////////////
void test_real() {
  int counter = 0;
  for (float i = -200000.123f; i < 200000.123f; i = i + 5.196f) {
    if (counter++ % 5000 == 0) {
      FOS_TEST_MESSAGE("Testing float conversion: %.4f", i);
    }
	bool flip = i > 0.0f;
    BinaryObj<> m = flip ? BinaryObj<>(i): BinaryObj<>::interpret(std::to_string(i) + "f");
    TEST_ASSERT_EQUAL(OType::REAL, m.type());
   TEST_ASSERT_FLOAT_WITHIN(0.1f, i, m.toReal().value());
    TEST_ASSERT_EQUAL(m.length(), strlen((const char *)m.data()));
    TEST_ASSERT_EQUAL_STRING(m.toString().c_str(), m.toStr().toString().c_str());
	TEST_ASSERT_TRUE(m.equals(BinaryObj<>::interpret(std::to_string(i) + "f")));
	//TEST_ASSERT_TRUE(m.equals(m.toStr().toReal()));
  }
}

 ////////////////////////////// STR ///////////////////////////////////
void test_str() {
 BinaryObj<> m =  BinaryObj<>("fhatty");
  TEST_ASSERT_EQUAL(OType::STR, m.type());
  TEST_ASSERT_EQUAL_STRING("fhatty", m.toStr().toString().c_str());
  TEST_ASSERT_EQUAL_STRING("fhatty", (const char *)m.data());
  TEST_ASSERT_EQUAL(6, m.length());
}

FOS_RUN_TESTS(                    //
    FOS_RUN_TEST(test_bool);      //
    FOS_RUN_TEST(test_int);       //
    FOS_RUN_TEST(test_real);     //
    FOS_RUN_TEST(test_str);       //
);

} // namespace fhatos

SETUP_AND_LOOP()

#endif
