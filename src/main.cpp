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

#include "fhatos.hpp"
#include "boot_loader.hpp"
#include "kernel.hpp"
#include "util/argv_parser.hpp"

#define HELP "  --!b%-15s!!= %5s\n"

using fhatos::ArgvParser;
using fhatos::BootLoader;
using fhatos::Ansi;
auto *args_parser = new ArgvParser();
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void setup() { BootLoader::primary_boot(args_parser)->done("main"); }

void loop() {
  // do nothing -- all looping handled by FhatOS scheduler
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#ifdef NATIVE
int main(const int argc, char **argv) {
  args_parser->init(argc, argv);
  if (args_parser->option_string("--help", "NO_HELP") != "NO_HELP") {
    const auto ansi = new Ansi();
    ansi->printf("%s: A Distributed Operating System\n", ansi->silly_print("FhatOS", true, true).c_str());
    ansi->printf(HELP, "help", "!rnoobj!!");
    ansi->printf(HELP, "ansi", "!gbool!!?!ycolorize!!");
    ansi->printf(HELP, "log", "!guri!!?{!gINFO!!,!yWARN!!,!rERROR!!,!mDEBUG!!,!cTRACE!!,!bALL!!,!cNONE!!}");
    ansi->printf(HELP, "fs:mount", "!guri!!?!ylocal_dir_path!!");
    ansi->printf(HELP, "mqtt:broker", "!guri!!?!yserver uri!!");
    ansi->printf(HELP, "mqtt:client", "!guri!!?!yclient_name!!");
    ansi->printf(HELP, "console:nest", "!gint!!?!ydepth!!");
    ansi->printf(HELP, "console:prompt", "!gstr!!?!yprompt_string!!");
    ansi->printf(HELP, "lib", "!guri!!?{!ymodule!!,!y...!!}");
    delete ansi;
  } else {
    setup();
    loop();
  }
  return 0;
}
#endif