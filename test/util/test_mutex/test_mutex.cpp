/*******************************************************************************
  FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_test_mutex_hpp
#define fhatos_test_mutex_hpp

#undef FOS_TEST_ON_BOOT

#include <test_fhatos.hpp>
#include FOS_PROCESS(thread.hpp)
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {

  ///////////////////// UTILITY THREAD

  struct Worker : public Thread {
    MutexDeque<int> *mutex;
    int counter = 0;

    Worker(int index, MutexDeque<int> *mutex) : Thread(ID(string("worker/").append(std::to_string(index)))) {
      this->mutex = mutex;
    }

    void setup() override {
      FOS_TEST_MESSAGE("%s up and running", this->id()->toString().c_str());
      TEST_ASSERT_FALSE(this->running());
      Thread::setup();
      TEST_ASSERT_TRUE(this->running());
    }

    void loop() override {
      TEST_ASSERT_TRUE(this->running());
      if (counter++ < 10) {
        this->mutex->push_back(counter);
      } else {
        this->stop();
        TEST_ASSERT_FALSE(this->running());
        FOS_TEST_MESSAGE("%s done and stopping", this->id()->toString().c_str());
      }
    }
  };

  ///////////////////// UTILITY METHOD

  void populateMutex(MutexDeque<int> *m, const BiConsumer<MutexDeque<int> *, int> &tester) {
    for (int i = 0; i < 100; i++) {
      tester(m, i);
    }
  }

  void test_mutex_deque_methods() {
    MutexDeque<int> m;
    ////// test push
    populateMutex(&m, [](MutexDeque<int> *m, int i) {
      m->push_front(i);
      TEST_ASSERT_EQUAL(i, m->pop_front().value());
      m->push_front(i);
      TEST_ASSERT_EQUAL(0, m->pop_back().value());
      m->push_back(0);
    });
    TEST_ASSERT_FALSE(m.empty());
    TEST_ASSERT_EQUAL(100, m.size());
    ////// test pop
    populateMutex(&m, [](MutexDeque<int> *m, int i) { TEST_ASSERT_EQUAL_INT32(i, m->pop_back().value()); });
    TEST_ASSERT_TRUE(m.empty());
    TEST_ASSERT_EQUAL(0, m.size());
    ////// test clear
    populateMutex(&m, [](MutexDeque<int> *m, int i) {
      m->push_back(i);
      TEST_ASSERT_EQUAL(i, m->pop_back().value());
      m->push_back(i);
      TEST_ASSERT_EQUAL(0, m->pop_front().value());
      m->push_front(0);
    });
    m.clear();
    TEST_ASSERT_TRUE(m.empty());
    TEST_ASSERT_EQUAL(0, m.size());
    ////// test remove_if
    populateMutex(&m, [](MutexDeque<int> *m, int i) {
      m->push_back(i);
      TEST_ASSERT_EQUAL(i, m->pop_back().value());
      m->push_back(i);
      TEST_ASSERT_EQUAL(0, m->pop_front().value());
      m->push_front(0);
    });
    m.remove_if([](int i) { return i % 2 == 0; });
    TEST_ASSERT_FALSE(m.empty());
    TEST_ASSERT_EQUAL(50, m.size());
    Scheduler::singleton()->barrier();
  }

  void test_mutex_deque_concurrently() {
    int WORKER_COUNT = 10;
    ptr<Scheduler> s = Scheduler::singleton();
    TEST_ASSERT_EQUAL(0, s->count("worker/+"));
    MutexDeque<int> m = MutexDeque<int>();
    TEST_ASSERT_EQUAL(0, m.size());
    TEST_ASSERT_TRUE(m.empty());
    for (int i = 0; i < WORKER_COUNT; i++) {
      TEST_ASSERT_TRUE(s->spawn((ptr<Thread>(new Worker(i, &m)))));
    }
    Scheduler::singleton()->barrier("no_workers", [s] { return s->count("worker/+") == 0; });
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
    s->barrier();
  }

  FOS_RUN_TESTS( //
          FOS_RUN_TEST(test_mutex_deque_methods); //
          FOS_RUN_TEST(test_mutex_deque_concurrently); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
