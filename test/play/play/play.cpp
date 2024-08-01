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

#ifndef fhatos_play_hpp
#define fhatos_play_hpp

#undef FOS_TEST_ON_BOOT

#include <language/obj.hpp>
#include <test_fhatos.hpp>

using namespace std;

namespace fhatos {

  class Play {
  public:
    Play() {
      GLOBAL_OPTIONS->LOGGING = LOG_TYPE::TRACE;
      GLOBAL_OPTIONS->PRINTING = Ansi<>::singleton();
      Obj_p s = /*Obj::to_bcode({});*/ Obj::to_rec({{"hi", 3}, {"bye", 4}, {"test",34}});
      Obj_p t = Obj::deserialize<Obj>(s->serialize());
      GLOBAL_OPTIONS->printer<>()->printf("%s and %s\n", s->toString().c_str(), t->toString().c_str());
      for (int i = 0; i < 1000; i++) {
        if (i % 100 == 0)
          printf("%i\n", i);
      }
    }
  };
} // namespace fhatos

int main(int argc, char **argv) {
  Play *p = new Play();
  delete p;
  return 1;
}
#endif
