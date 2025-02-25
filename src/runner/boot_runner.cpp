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

#ifndef fhatos_boot_runner_cpp
#define fhatos_boot_runner_cpp

#include <chrono>
#include "../../../src/fhatos.hpp"
#include "../../../src/kernel.hpp"
#include "../../../src/util/ansi.hpp"
#include "../../../src/boot_loader.hpp"

using namespace fhatos;
using namespace std;

int main(int, char **) {
  try {
    char** args = new char*();
    args[0] = static_cast<char *>("boot_runner");
    args[1] = static_cast<char *>("--headers=true");
    args[2] = static_cast<char *>("--log=INFO");
    args[3] = static_cast<char *>("--ansi=false");
    args[4] = static_cast<char *>("--boot:config=../../../conf/boot_config.obj");
    ArgvParser* argv_parser = new ArgvParser();
    argv_parser->init(5,args);
    printer()->ansi_switch(false);
    cout << "++++\n[source,mmadt,subs=\"verbatim\"]\n----\n";
    BootLoader::primary_boot(argv_parser)
        ->display_splash("----\n")
        ->display_splash("++++");
    cout << "\n----\n++++";
    return 0;
  } catch (const std::exception &e) {
    throw;
  }
}
#endif
