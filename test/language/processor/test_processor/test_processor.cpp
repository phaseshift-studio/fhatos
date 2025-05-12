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

#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_SHARED_MEMORY
#define FOS_DEPLOY_PROCESSOR

#include "../../../test_fhatos.hpp"

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  using P = fhatos::Processor;

  void test_monad_set() {
    LOG(INFO, "creating processor\n");
    ptr<Processor> processor = make_shared<Processor>(__(jnt(14)));
    LOG(INFO, "processor created\n");
    P::MonadSet mset = processor->make_monad_set();
    //  LOG(INFO,"monad set created\n");
    P::Monad_p m1 = processor->make_monad(jnt(10), noobj());
    P::Monad_p m2 = processor->make_monad(jnt(20), noobj());
    P::Monad_p m3 = processor->make_monad(jnt(10), noobj());
    FOS_TEST_OBJ_EQUAL(m1, m3);
    FOS_TEST_OBJ_NTEQL(m1, m2);
    //LOG(INFO,"monads created\n");
    TEST_ASSERT_EQUAL_INT(0, mset.size());
    mset.push_back(m1);
    TEST_ASSERT_EQUAL_INT(1, mset.size());
    TEST_ASSERT_EQUAL_INT(1, mset.bulk_of(m1));
    FOS_TEST_OBJ_EQUAL(m1, mset.front());
    TEST_ASSERT_EQUAL_INT(1, mset.front()->bulk);
    TEST_ASSERT_EQUAL_INT(1, mset.size());
    mset.push_back(m2);
    TEST_ASSERT_EQUAL_INT(2, mset.size());
    TEST_ASSERT_EQUAL_INT(1, mset.bulk_of(m1));
    mset.push_back(m3);
    TEST_ASSERT_EQUAL_INT(2, mset.bulk_of(m1));
    TEST_ASSERT_EQUAL_INT(2, mset.bulk_of(m3));
    TEST_ASSERT_EQUAL_INT(2, mset.size());
    int total_runs = 20;
    for(int i = 1; i < total_runs; i++) {
      mset.push_back(m3);
      FOS_TEST_OBJ_EQUAL(m1, mset.front());
      TEST_ASSERT_EQUAL_INT(2+i, mset.bulk_of(m1));
      TEST_ASSERT_EQUAL_INT(2, mset.size());
    }
    P::Monad_p m4 = processor->make_monad(jnt(10), __().from("abc"));
    m4->bulk = 1000;
    mset.push_back(m4);
    TEST_ASSERT_EQUAL_INT(1+total_runs, mset.bulk_of(m1));
    TEST_ASSERT_EQUAL_INT(1+total_runs, mset.bulk_of(m3));
    TEST_ASSERT_EQUAL_INT(3, mset.size());
    TEST_ASSERT_EQUAL_INT(1000, mset.bulk_of(m4));
    //////
    mset.pop_front();
    TEST_ASSERT_EQUAL_INT(2, mset.size());
    mset.pop_front();
    TEST_ASSERT_EQUAL_INT(1, mset.size());
    mset.pop_front();
    TEST_ASSERT_TRUE(mset.empty());
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_monad_set); //
      )
}; // namespace fhatos

SETUP_AND_LOOP();
