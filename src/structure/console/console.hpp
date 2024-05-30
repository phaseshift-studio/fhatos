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

#ifndef fhatos_console_hpp
#define fhatos_console_hpp

#include <fhatos.hpp>
#include <iostream>
#include <util/ansi.hpp>
#include <structure/furi.hpp>
#include <language/parser.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  template<typename PRINTER>
  class Console final : public Thread {
  public:
    Ansi<PRINTER> *ansi;
    Parser *parser;

    explicit Console(const ID &id = ID("anon")): Thread(id) {
    }

    void setup() override {
      Thread::setup();
      _logging = LOG_TYPE::ERROR;
      this->ansi = new Ansi<PRINTER>(PRINTER::singleton());
      this->parser = new Parser(this->id());
    }

    void loop() override {
      string line = "";
      while (true) {
        this->ansi->printf("%s", prompt());
        line.clear();
        std::getline(std::cin, line);
        if (line == ":quit") {
          this->stop();
          return;
        }
        ptr<Fluent<> > f = this->parser->parseToFluent(line.c_str());
        f->forEach<Obj>([this](const Obj *obj) {
          this->ansi->printf("%s%s\n", result(), obj->toString().c_str());
        });
      }
    }

    const char *prompt() const {
      return "!mfhatos!!> ";
    }

    const char *result() const {
      return "!g==!!>";
    }
  };
}

#endif
