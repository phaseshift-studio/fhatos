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

#pragma once
#ifndef fhatos_terminal_hpp
#define fhatos_termainl_hpp

#include <fhatos.hpp>
#include <iostream>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  class Terminal : public Actor<Thread> {
  protected:
    ID_p _currentOutput;

    explicit Terminal(const ID &id = ID("/io/terminal/")) :
        Actor(id,
              [](Actor *actor) {
                actor->subscribe("out", [actor](const Message_p &message) {
                  if (Terminal::singleton()->_currentOutput->equals(message->source)) {
                    const string copy = string(message->payload->str_value());
                    if (message->target.name() == "no_color")
                      GLOBAL_OPTIONS->printer<>()->print(GLOBAL_OPTIONS->printer<>()->strip(copy.c_str()));
                    else
                      GLOBAL_OPTIONS->printer<>()->print(copy.c_str());
                  }
                });
                actor->subscribe("in", [actor](const Message_p &message) {
                  string line;
                  std::getline(std::cin, line);
                  actor->publish(message->source, Obj::to_str(line));
                });
              }),
        _currentOutput(share(id)) {}

  public:
    static Terminal *singleton(const ID &id = "/io/terminal/") {
      static Terminal terminal = Terminal(id);
      return &terminal;
    }

    static ID_p currentOut() { return Terminal::singleton()->_currentOutput; }

    static void currentOut(const ID_p &source) { Terminal::singleton()->_currentOutput = source; }

    template<typename PRINTER = Ansi<>>
    static PRINTER *printer() {
      return GLOBAL_OPTIONS->printer<>();
    }

    static int readChar() {
#ifdef NATIVE
      return getchar();
#else
      return (Serial.available() > 0) ? Serial.read() : EOF;
#endif
    }

    static void out(const ID &source, const char *format, ...) {
      char buffer[512];
      va_list arg;
      va_start(arg, format);
      int length = vsnprintf(buffer, 512, format, arg);
      buffer[length] = '\0';
      va_end(arg);
      GLOBAL_OPTIONS->router<Router>()->publish(Message{.source = source, //
                                                        .target = Terminal::singleton()->id()->extend("out"), //
                                                        .payload = Obj::to_str(buffer),
                                                        .retain = TRANSIENT_MESSAGE});
    }

    static string in(const ID &source) {
      /* GLOBAL_OPTIONS->router<Router>()->publish(Message{.source = source, //
                                                         .target = Terminal::singleton()->id()->extend("/out"), //
                                                         .payload = Obj::to_str(toPrint)});*/
      return "todo";
    }
  };
} // namespace fhatos
#endif
