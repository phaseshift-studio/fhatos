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
#define fhatos_terminal_hpp

#include <fhatos.hpp>
#include <iostream>
#include <process/actor/actor.hpp>
#include <structure/stype/key_value.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  class Terminal : public Actor<Thread, KeyValue> {
  protected:
    explicit Terminal(const ID &id = ID("/io/terminal/")) : Actor(id), _currentOutput(share(id)) {}
    ID_p _currentOutput;

  public:
    static Terminal *singleton(const ID &id = "/io/terminal/") {
      static Terminal terminal = Terminal(id);
      return &terminal;
    }
    void setup() override {
      Actor::setup();
      this->subscribe("out", [](const Message_p &message) {
        if (message->source.matches(*Terminal::singleton()->_currentOutput)) {
          if (strcmp(message->target.name(), "no_color") == 0) {
            const string no = Options::singleton()->printer<>()->strip(message->payload->str_value());
            Options::singleton()->printer<>()->print(no.c_str());
          } else
            Options::singleton()->printer<>()->print(message->payload->str_value().c_str());
        }
      });
      this->subscribe("in", [this](const Message_p &message) {
        string line;
        std::getline(std::cin, line);
        this->publish(message->source, Obj::to_str(line));
      });
    }

    static ID_p currentOut() { return Terminal::singleton()->_currentOutput; }

    static void currentOut(const ID_p &source) { Terminal::singleton()->_currentOutput = source; }

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
      Options::singleton()->rooter<Rooter>()->route_message(
          share(Message{.source = source, //
                        .target = Terminal::singleton()->id()->extend("out"), //
                        .payload = Obj::to_str(buffer),
                        .retain = TRANSIENT_MESSAGE}));
    }

    static string in(const ID &) {
      /* GLOBAL_OPTIONS->router<Router>()->publish(Message{.source = source, //
                                                         .target = Terminal::singleton()->id()->extend("/out"), //
                                                         .payload = Obj::to_str(toPrint)});*/
      return "todo";
    }
  };
} // namespace fhatos
#endif
