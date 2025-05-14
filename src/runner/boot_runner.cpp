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

#include "../../../src/fhatos.hpp"
#include "../../../src/kernel.hpp"
#include "../../../src/util/ansi.hpp"
#include "../../../src/boot_loader.hpp"

using namespace fhatos;
using namespace std;

int main(int, char **) {
  try {
    char* args[5];
    args[0] = "boot_runner";
    args[1] ="--headers=true";
    args[2] = "--log=INFO";
    args[3] = "--ansi=false";
    args[4] = "--boot:config=/boot/boot_config.obj";
    auto* argv_parser = new ArgvParser();
    argv_parser->init(5,args);
    printer()->ansi_switch(false);
    BootLoader::primary_boot(argv_parser);
    std::this_thread::sleep_for(chrono::milliseconds(1000));
    return 0;
  } catch (const std::exception &e) {
    LOG_WRITE(ERROR,Router::singleton().get(),L("{}",e.what()));
    throw;
  }
}
#endif
