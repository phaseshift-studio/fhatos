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
#include <util/options.hpp>
#include <util/ansi.hpp>
#include <fhatos.hpp>
#include <kernel.hpp>
#include <language/insts.hpp>
#include <language/parser.hpp>
#include <language/type.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include <process/ptype/native/scheduler.hpp>
#include <thread>
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
  Options::singleton()->printer<Ansi<>>(Ansi<>::singleton());
  try{
    char **args = new char *();
      args[0] = (char *) "main_runner";
      args[1] = (char *) "--headers=false";
      args[2] = (char *) "--log=ERROR";
      args[3] = (char *) "--ansi=false";
      ArgvParser * argv_parser = new ArgvParser();
      argv_parser->init(4, args);
      Options::singleton()->printer<Ansi<>>()->on(false);
      BootLoader::primary_boot(argv_parser);
  }
  catch(const std::exception & e) {
    throw;
  }
  LOG(INFO, "Processing %s\n", argsv[1]);
  printer<>()->println("++++\n[source,mmadt]\n----");
  //router()->write(id_p("/console/:prompt"), str(""), false);
  //router()->loop();
  for (int i = 1; i < arg; i++) {
    try{
      std::string x = argsv[i];
        StringHelper::trim(x);
        /* if(x.find("!NO!") != std::string::npos) {
               printer<Ansi<>>()->on(false);
               router()->write(id_p("/console/:prompt"), str(x.substr(5)), false);
               printer<Ansi<>>()->on(true);
           }
   */if (x.find("/console/config/nest ->") != string::npos) {
            router()->write(id_p("/console/config/nest"), jnt(stoi(x.substr(x.find("->")+2))));
        }else {
            router()->write(id_p("/console/:prompt"), str(x), false);
        }
        router()->loop();
    }
    catch(std::exception & e) {
      LOG_EXCEPTION(Scheduler::singleton(),e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printer<>()->print("----\n++++");
  return 0;
}
#endif