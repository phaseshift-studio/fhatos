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
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PROCESSOR
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"
#include STR(../../../../src/structure/stype/fs/HARDWARE/fs.hpp)
#include "../generic_structure_test.hpp"

namespace fhatos {
  using namespace mmadt;

  Structure_p test_fs = std::make_shared<FSx>("/fs/xyz/#",id_p("/sys/test"),Obj::to_rec({{"root",vri("./fs")}}));
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////

  void test_fs_generic_write() { GenericStructureTest(test_fs).test_write(); }
  void test_fs_generic_subscribe() { GenericStructureTest(test_fs).test_subscribe(); }
  void test_fs_generic_lst_embedding() { GenericStructureTest(test_fs).test_lst_embedding(); }
  void test_fs_generic_rec_embedding() { GenericStructureTest(test_fs).test_rec_embedding(); }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_fs_generic_write); //
      FOS_RUN_TEST(test_fs_generic_subscribe); //
      FOS_RUN_TEST(test_fs_generic_lst_embedding); //
      FOS_RUN_TEST(test_fs_generic_rec_embedding); //
      );

} // namespace fhatos

SETUP_AND_LOOP();
