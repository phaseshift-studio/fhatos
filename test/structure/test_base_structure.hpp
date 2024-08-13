//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef fhatos_base_structure_hpp
#define fhatos_base_structure_hpp

#include <fhatos.hpp>
#include <structure/router.hpp>
#include <structure/structure.hpp>
#include <test_fhatos.hpp>


#define FOS_STOP_ON_BOOT  \
router()->detach(current_structure->pattern()); \

namespace fhatos {
ptr<Structure> current_structure;

void test_write() {
    router()->attach(current_structure);
    FOS_TEST_EXCEPTION_CXX(router()->write(id_p("/a/b/c"), noobj(), id_p("fhatty")));
}



}// namespace fhatos

#endif
