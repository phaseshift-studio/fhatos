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
#include "../../../src/lang/mmadt/type.hpp"
#include "../../../src/lang/mmadt/parser.hpp"
#include "../../../src/lang/type.hpp"
#include "../../../src/model/console.hpp"
#include "../../../src/model/terminal.hpp"
#include "../../../src/process/ptype/native/scheduler.hpp"
#include <thread>
#include "../../../src/util/ansi.hpp"
#include "../../../src/util/print_helper.hpp"
#include "../../../src/util/options.hpp"
#include "../../../src/boot_loader.hpp"

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
      args[2] = (char *) "--log=INFO";
      args[3] = (char *) "--ansi=false";
      args[4] = (char *) "--boot:config=../../../conf/boot_config.obj";
      ArgvParser * argv_parser = new ArgvParser();
      argv_parser->init(5, args);
      Options::singleton()->printer<Ansi<>>()->on(true);
      BootLoader::primary_boot(argv_parser);
  }
  catch(const std::exception & e) {
    fhatos::LOG(ERROR,"error occurred processing docs: %s",e.what());
    throw;
  }
  LOG(INFO, "Processing %s\n", argsv[1]);
  printer<>()->println("++++\n[source,mmadt]\n----");
  //router()->write(id_p("/console/:prompt"), str(""), false);
  //router()->loop();
  for (int i = 1; i < 2; i++) {
    try{
      std::string x = string(argsv[i]);
        StringHelper::trim(x);
        /* if(x.find("!NO!") != std::string::npos) {
               printer<Ansi<>>()->on(false);
               router()->write(id_p("/console/:prompt"), str(x.substr(5)), false);
               printer<Ansi<>>()->on(true);
           }
   if (x.find("/io/console/config/nest ->") != string::npos) {
            Router::singleton()->write(id_p("/io/console/config/nest"), jnt(stoi(x.substr(x.find("->")+2))));
        }else {
            Router::singleton()->write(id_p("/io/console/:prompt"), str(x), false);
        }*/
        printer<>()->print(BCODE_PROCESSOR(OBJ_PARSER(x))->to_objs()->toString().c_str());
        //Router::singleton()->loop();
    }
    catch(std::exception & e) {
      LOG_EXCEPTION(Scheduler::singleton(),e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printer<>()->print("----\n++++");
}
#endif