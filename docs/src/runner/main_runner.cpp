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

#ifndef fhatos_main_runner_cpp
#define fhatos_main_runner_cpp

#include <chrono>
#include <thread>
#include "../../../src/boot_loader.hpp"
#include "../../../src/fhatos.hpp"
#include "../../../src/kernel.hpp"
#include "../../../src/lang/mmadt/mmadt_obj.hpp"
#include "../../../src/lang/mmadt/parser.hpp"
#include "../../../src/lang/processor/processor.hpp"
#include "../../../src/model/fos/sys/scheduler/scheduler.hpp"
#include "../../../src/model/fos/sys/typer/typer.hpp"
#include "../../../src/model/fos/ui/console.hpp"
#include "../../../src/model/fos/ui/terminal.hpp"
#include "../../../src/util/ansi.hpp"
#include "../../../src/util/options.hpp"
#include "../../../src/util/print_helper.hpp"

using namespace fhatos;
using namespace mmadt;

void printResult(const Obj_p &obj, const uint8_t depth = 0) {
  if(obj->is_noobj())
    return;
  if(obj->is_objs()) {
    for(Obj_p &o: *obj->objs_value()) {
      printResult(o, depth + 1);
    }
  } else {
    printer<>()->printf("!g==>!!%s\n", obj->toString().c_str());
  }
}

int main(int arg, char **argsv) {
  Options::singleton()->printer<Ansi<>>(Ansi<>::singleton());
  try {
    char *args[5];
    args[0] = "main_runner";
    args[1] = "--headers=false";
    args[2] = "--log=INFO";
    args[3] = "--ansi=false";
    args[4] = "--boot:config=/boot/boot_config.obj";
    ArgvParser *argv_parser = new ArgvParser();
    argv_parser->init(5, args);
    printer()->printer_switch(false);
    printer()->ansi_switch(false);
    BootLoader::primary_boot(argv_parser);
    Router::singleton()->write("/io/console/config/terminal", Obj::to_noobj(), true);
    Router::singleton()->write("/io/console/config/stack_trace", dool(false), true);

    LOG(INFO, "processing %i expressions\n", arg);
    const Console *console = Obj::to_rec(rmap({}),CONSOLE_FURI,id_p("/io/console"))->get_model<Console>();
    Router::singleton()->loop();
    printer()->printer_switch(true);
    for(int i = 1; i < arg; i++) {
      try {
        auto x = string(argsv[i]);
        StringHelper::trim(x);
        const bool has_thread = x.find("spawn") != string::npos || x.find("halt") != string::npos;
        printer()->print(Router::singleton()->read("/io/console/config/prompt")->str_value().c_str());
        printer()->println(x.c_str());
        Router::singleton()->loop();
        // StringHelper::replace(&x, "'", "\\'"); // escape quotes
        // StringHelper::replace(&x, "{", "{");
        // StringHelper::replace(&x, "}", "}");
        console->process_line(x);
        // TODO: using this access point as issues with {} in print() being intercepted by fmt package
        // Processor::compute(string("*/io/console.eval('").append(x).append("')"));
        Router::singleton()->loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(has_thread ? 2500 : 10));
        Router::singleton()->write(SCHEDULER_ID->extend("halt"), dool(true));
      } catch(const fError &e) {
        printer()->print("[ERROR] ");
        printer()->println(e.what());
      }
    }
  } catch(const std::exception &e) {
    printer()->printer_switch(true);
    fhatos::LOG(ERROR, "error occurred processing docs: %s", e.what());
    throw;
  }
  printer()->printer_switch(false);

  // Scheduler::singleton()->stop();
  return 0;
}
#endif
