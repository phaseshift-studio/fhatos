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

#ifndef fhatos_test_process_hpp
#define fhatos_test_process_hpp


#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_MMADT_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_FOS_TYPE
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /process/#
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"


namespace fhatos {

  void test_basic_thread() {

  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_basic_thread); //
      );

} // namespace fhatos

SETUP_AND_LOOP();

#endif