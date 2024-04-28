#ifndef fhatos_kernel__test_mutex_hpp
#define fhatos_kernel__test_mutex_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/util/mutex/mutex_deque.hpp>
#include <unity.h>

namespace fhatos::kernel {

void test_deque() {
  MutexDeque<int> m = MutexDeque<int>();
  TEST_ASSERT_TRUE(m.empty());
  TEST_ASSERT_EQUAL(0, m.size());
  for (int i = 0; i < 100; i++) {
    m.push_front(i);
  }
  TEST_ASSERT_FALSE(m.empty());
  TEST_ASSERT_EQUAL(100, m.size());
  for (int i = 0; i < 100; i++) {
    TEST_ASSERT_EQUAL_INT32(i, m.pop_back().value());
  }
  TEST_ASSERT_TRUE(m.empty());
  TEST_ASSERT_EQUAL(0, m.size());
}

FOS_RUN_TESTS(                //
    FOS_RUN_TEST(test_deque); //
);

} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif