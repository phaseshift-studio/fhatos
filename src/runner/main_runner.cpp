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
#include <fhatos.hpp>
#include <kernel.hpp>
#include <language/insts.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include <process/ptype/native/scheduler.hpp>
#include <thread>
#include <util/ansi.hpp>
#include <util/options.hpp>
#include <boot_loader.hpp>

using namespace fhatos;

void printResult(const Obj_p &obj, const uint8_t depth = 0) {
  if (obj->is_noobj())
    return;
  if (obj->is_objs()) {
    for (Obj_p &o: *obj->objs_value()) {
      printResult(o, depth + 1);
    }
  } else {
    printer<>()->printf("!g==>!!%s\n", obj->toString().c_str());
  }
}

int main(int arg, char **argsv) {
  try {
    char** args = new char*();
    args[0] = (char*) "main_runner";
    args[1] = (char*) "--headers=false";
    args[2] = (char*) "--log=ERROR";
    args[3] = (char*) "--ansi=false";
    ArgvParser* argv_parser = new ArgvParser();
    argv_parser->init(4,args);
    BootLoader::primary_boot(argv_parser);
    Options::singleton()->printer<Ansi<>>()->on(false);
  } catch (const std::exception &e) {
    throw;
  }
  LOG(INFO, "Processing %s\n", argsv[1]);
  printer<>()->println("++++\n[source,mmadt]\n----");
  for (int i = 1; i < arg; i++) {
    try {
      string x = argsv[i];
      StringHelper::trim(x);
      if(x == "/console/config/nest -> true") {
         router()->write(id_p("/console/config/nest"),dool(true));
      } else {
      //printer<>()->printf("%s\n", x.c_str());
      router()->write(id_p("/console/prompt"),str(x),false);
      router()->loop();
      }
      /*printer<>()->printf("fhatos> %s\n", x.c_str());
      const Option<Obj_p> obj = Parser::singleton()->try_parse_obj(argsv[i]);
      if (obj.has_value()) {
        //printResult(Options::singleton()->processor<Obj,BCode,Obj>(noobj(),obj.value()));
        printResult(Fluent(obj.value()).toObjs());
      }*/
    } catch (std::exception &e) {
      LOG_EXCEPTION(e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printer<>()->print("\n----\n++++");
  return 0;
}
#endif
