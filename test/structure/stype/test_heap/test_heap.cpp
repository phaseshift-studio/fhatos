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

#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"
#include "../generic_structure_test.hpp"

namespace fhatos {
  using namespace mmadt;

  const Structure_p test_structure = std::make_shared<Heap<>>("/xyz/#", id_p("/sys/test"));
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////
  void test_generic_clear() { GenericStructureTest(test_structure).test_clear(); }

  void test_generic_write() { GenericStructureTest(test_structure).test_write(); }

  void test_generic_delete() { GenericStructureTest(test_structure).test_delete(); }

  void test_generic_subscribe() { GenericStructureTest(test_structure).test_subscribe(); }

  void test_generic_mono_embedding() { GenericStructureTest(test_structure).test_mono_embedding(); }

  void test_generic_lst_embedding() { GenericStructureTest(test_structure).test_lst_embedding(); }

  void test_generic_rec_embedding() { GenericStructureTest(test_structure).test_rec_embedding(); }

  void test_generic_q_sub() { GenericStructureTest(test_structure).test_q_sub(); }

  void test_generic_q_doc() { GenericStructureTest(test_structure).test_q_doc(); }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_generic_clear); //
      FOS_RUN_TEST(test_generic_write); //
      FOS_RUN_TEST(test_generic_delete); //
      FOS_RUN_TEST(test_generic_subscribe); //
      FOS_RUN_TEST(test_generic_mono_embedding); //
      FOS_RUN_TEST(test_generic_lst_embedding); //
      FOS_RUN_TEST(test_generic_rec_embedding); //
      FOS_RUN_TEST(test_generic_q_sub); //
      FOS_RUN_TEST(test_generic_q_doc); //
  );

} // namespace fhatos

SETUP_AND_LOOP();
