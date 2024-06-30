#ifndef fhatos_main_runner_cpp
#define fhatos_main_runner_cpp

#include <fhatos.hpp>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <util/ansi.hpp>

using namespace fhatos;
int main(int arg, char **argsv) {
  Fluent(Parser::tryParseObj("__(1,2,3).plus(10)").value()).forEach<Obj>([](const Obj_p o) {
    Ansi<CPrinter>::singleton()->on(false);
   printf("==>%s\n",  Ansi<CPrinter>::singleton()->strip(o->toString().c_str()));
  });
  return 1;
}
#endif
