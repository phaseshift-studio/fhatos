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
#include "../../../src/fhatos.hpp"
#include "../../../src/kernel.hpp"
#include "../../../src/lang/mmadt/mmadt_obj.hpp"
#include "../../../src/lang/mmadt/parser.hpp"
#include "../../../src/lang/processor/processor.hpp"
#include "../../../src/lang/type.hpp"
#include "../../../src/model/fos/ui/console.hpp"
#include "../../../src/model/fos/ui/terminal.hpp"
#include "../../../src/model/fos/sys/scheduler/fscheduler.hpp"
#include <thread>
#include "../../../src/util/ansi.hpp"
#include "../../../src/util/print_helper.hpp"
#include "../../../src/util/options.hpp"
#include "../../../src/boot_loader.hpp"

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
    const auto args = new char *();
    args[0] = static_cast<char *>("main_runner");
    args[1] = static_cast<char *>("--headers=false");
    args[2] = static_cast<char *>("--log=INFO");
    args[3] = static_cast<char *>("--ansi=false");
    args[4] = static_cast<char *>("--boot:config=../../../conf/boot_config.obj");
    ArgvParser *argv_parser = new ArgvParser();
    argv_parser->init(5, args);
    printer()->printer_switch(false);
    printer()->ansi_switch(false);
    BootLoader::primary_boot(argv_parser);
    Router::singleton()->write("/io/console/config/terminal",Obj::to_noobj(),true);
  } catch(const std::exception &e) {
    fhatos::LOG(ERROR, "error occurred processing docs: %s", e.what());
    throw;
  }
  LOG(INFO, "Processing %i expressions\n", arg);
  printer()->printer_switch(true);
  Router::singleton()->loop();
  for(int i = 1; i < arg; i++) {
    try {
      auto x = string(argsv[i]);
      StringHelper::trim(x);
      printer()->print(Router::singleton()->read("/io/console/config/prompt")->str_value().c_str());
      // printer()->print(" ");
      printer()->println(x.c_str());
      Router::singleton()->loop();
      StringHelper::replace(&x, "'", "\\'"); // escape quotes
      Processor::compute(fmt::format("*/io/console.eval('{}')", x));
      Router::singleton()->loop();
      if(x.find("spawn") != string::npos)
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    } catch(std::exception &e) {
      LOG_EXCEPTION(fScheduler::singleton(), e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  delete[] argsv;
  return 0;
}
#endif
