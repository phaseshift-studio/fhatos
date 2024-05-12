#ifndef fhatos_test_native_hpp
#define fhatos_test_native_hpp
#include <unity.h>

namespace fhatos {

void test_abc() {
TEST_ASSERT_EQUAL(2,2);
}

  void RUN_UNITY_TESTS() {                                                     \
                                    \
  UNITY_BEGIN();                                                             \
  RUN_TEST(test_abc);                                                                 \
  UNITY_END();                                                               \
}




}

int main(int arg, char **argsv) {
fhatos::RUN_UNITY_TESTS();
}

#endif