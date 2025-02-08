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

#pragma once
#include "../../../src/fhatos.hpp"
#include "../../test_fhatos.hpp"
#include "../../../src/structure/structure.hpp"

namespace fhatos {

  class GenericStructureTest {
protected:
     const Structure_p structure_;
     const ID_p prefix_;
public:
     explicit GenericStructureTest(const Structure_p& structure):
      structure_(structure),
      prefix_(id_p(structure->pattern->retract_pattern())) {
       Router::singleton()->attach(structure);

     };

   [[nodiscard]] ID_p p(const fURI& furi) const {
     return id_p(prefix_->extend(furi));
   };

   void test_write() const {
    for(int i=0;i<50;i++) {
     structure_->write(p(string("a").append(to_string(i))),jnt(i*10));
     }

     for(int i=0;i<50;i++) {
       TEST_ASSERT_EQUAL_INT(i*10,structure_->read(p(string("a").append(to_string(i))))->int_value());
      }
    };
};
}