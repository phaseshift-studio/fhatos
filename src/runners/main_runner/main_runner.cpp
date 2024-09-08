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
#include <language/insts.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include <process/ptype/native/scheduler.hpp>
#include <kernel.hpp>
#include <model/console.hpp>
#include <model/terminal.hpp>
#include <thread>
#include <util/ansi.hpp>

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
    Kernel::build()
            ->using_printer(Ansi<>::singleton())
            ->with_log_level(ERROR)
            ->using_scheduler(Scheduler::singleton("/sys/scheduler/"))
            ->using_router(Router::singleton("/sys/router/"))
            ->boot<Terminal>(Terminal::singleton("/io/terminal/"))
            ->boot<Types>(Types::singleton("/type/"))
            ->boot<Parser>(Parser::singleton("/sys/lang/parser/"))
            ->boot<Console>(ptr<Console>(Console::create("/home/root/repl/")))
                    //->boot<FileSystem, Fiber, Mount>(FileSystem::singleton("/io/fs"))
           // ->model({ID("/model/proc")})
        ->initial_terminal_owner("/home/root/repl/");
    //->done("kernel_barrier");
    printer<>()->on(false);
  } catch (const std::exception &e) {
    throw;
  }
  LOG(INFO, "Processing %s\n", argsv[1]);
  printer<>()->println("++++\n[source,mmadt]\n----");
  for (int i = 1; i < arg; i++) {
    try {
      string x = argsv[i];
      StringHelper::trim(x);
      printer<>()->printf("fhatos> %s\n", x.c_str());
      const Option<Obj_p> obj = Parser::singleton()->tryParseObj(argsv[i]);
      if (obj.has_value()) {
        printResult(Fluent(obj.value()).toObjs());
      }
    } catch (std::exception &e) {
      LOG_EXCEPTION(e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  printer<>()->print("----\n++++");
  // Scheduler::singleton()->stop();
  return 0;
}
#endif
