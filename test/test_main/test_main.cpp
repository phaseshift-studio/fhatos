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

#ifndef fhatos_test_main_cpp
#define fhatos_test_main_cpp

//#include <main.cpp>
namespace fhatos {
  static int test_boot() {
    char **args = new char *[1]();
    args[0] = (char *) "fhatos";
    args[1] = (char *) "--barrier=false";
    //int result = main(2, args);
    int result = 0;
    delete[] args;
    return result;
  }
}

int main(const int, char **) {
  return fhatos::test_boot();
}

#endif