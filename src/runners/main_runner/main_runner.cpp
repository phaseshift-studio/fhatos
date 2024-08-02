#ifndef fhatos_main_runner_cpp
#define fhatos_main_runner_cpp

#include <fhatos.hpp>
#include <chrono>
#include <thread>
#include <language/parser.hpp>
#include <util/ansi.hpp>
#include <language/insts.hpp>
#include <language/types.hpp>
#include <process/native/scheduler.hpp>
#include <structure/console/console.hpp>
#include <structure/io/terminal.hpp>
#include <structure/kernel.hpp>

using namespace fhatos;

void printResult(const Obj_p &obj, const uint8_t depth = 0) {
  if(obj->isNoObj())
    return;
  if (obj->isObjs()) {
    for (Obj_p &o: *obj->objs_value()) {
      printResult(o, depth + 1);
    }
  } else {
     Options::singleton()->printer<>()->printf("!g==>!!%s\n", obj->toString().c_str());
  }
}

int main(int arg, char **argsv) {
  try {
    Kernel::build()
        ->initialPrinter(Ansi<>::singleton())
        ->initialLogLevel(ERROR)
        ->onBoot(Scheduler::singleton("/sys/scheduler/"), //
                 {LocalRouter::singleton(), //
                  Terminal::singleton("/sys/io/terminal/"), //
                  Types::singleton("/sys/lang/type/"), //
                  Parser::singleton("/sys/lang/parser/"), //
                  new Console("/home/root/repl/")})
        ->loadModules({"/ext/process"})
        ->defaultOutput("/home/root/repl/"); // ->done("kernel_barrier");
     Options::singleton()->printer<>()->on(false);
  } catch (const std::exception &e) {
    throw;
  }
  LOG(INFO, "Processing %s\n", argsv[1]);
   Options::singleton()->printer<>()->println("++++\n[source,mmadt]\n----");
  for (int i = 1; i < arg; i++) {
    try {
      string x = argsv[i];
      StringHelper::trim(x);
       Options::singleton()->printer<>()->printf("fhatos> %s\n", x.c_str());
      const Option<Obj_p> obj = Parser::singleton()->tryParseObj(argsv[i]);
      if(obj.has_value()) {
        printResult(Fluent(obj.value()).toObjs());
      }
    } catch (std::exception &e) {
      LOG_EXCEPTION(e);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
   Options::singleton()->printer<>()->print("----\n++++");
  //Scheduler::singleton()->stop();
  return 0;
}
#endif
