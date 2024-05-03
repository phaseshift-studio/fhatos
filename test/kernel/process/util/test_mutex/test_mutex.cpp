#ifndef fhatos_kernel__test_mutex_hpp
#define fhatos_kernel__test_mutex_hpp

#include <test_fhatos.hpp>
//
#include <kernel/process/esp32/scheduler.hpp>
#include <kernel/util/mutex_deque.hpp>
#include <unity.h>

namespace fhatos::kernel {

struct Worker : public Thread {
  MutexDeque<int> *mutex;
  int counter = 0;
  Worker(int index, MutexDeque<int> *mutex)
      : Thread(ID(String("worker/") + index)) {
    this->mutex = mutex;
  }
  void setup() {
    FOS_TEST_MESSAGE("%s up and running", this->id().toString().c_str());
    TEST_ASSERT_TRUE(this->running());
  }

  void loop() {
    TEST_ASSERT_TRUE(this->running());
    if (counter++ < 10) {
      this->mutex->push_back(counter);
    } else {
      this->stop();
      TEST_ASSERT_FALSE(this->running());
      FOS_TEST_MESSAGE("%s done and stopping", this->id().toString().c_str());
    }
  }
};

void test_threaded() {
  int WORKER_COUNT = 10;
  Scheduler<> *s = Scheduler<>::singleton();
  TEST_ASSERT_EQUAL(0, s->count("worker/+"));
  MutexDeque<int> m = MutexDeque<int>();
  TEST_ASSERT_EQUAL(0, m.size());
  TEST_ASSERT_TRUE(m.empty());
  for (int i = 0; i < WORKER_COUNT; i++) {
    TEST_ASSERT_TRUE(s->spawn(new Worker(i, &m)));
  }
  while (s->count("worker/+") > 0) {
    // delay(1000);
  }
  TEST_ASSERT_EQUAL(0, s->count("worker/+"));
  TEST_ASSERT_EQUAL(10 * WORKER_COUNT, m.size());
  TEST_ASSERT_FALSE(m.empty());
  int sum = 0;
  for (int i = 1; i < 11; i++) {
    sum += i;
  }
  sum = sum * WORKER_COUNT;
  int mutexSum = 0;
  int temp = 0;
  while (-1 != temp) {
    mutexSum += temp;
    temp = m.pop_front().value_or(-1);
  }
  TEST_ASSERT_EQUAL(sum, mutexSum);
}

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

FOS_RUN_TESTS(                   //
    FOS_RUN_TEST(test_deque);    //
    FOS_RUN_TEST(test_threaded); //
);
} // namespace fhatos::kernel

SETUP_AND_LOOP()

#endif