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
#include <language/parser.hpp>
#include <structure/furi.hpp>
#include <util/ansi.hpp>
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  template<typename PRINTER>
  class Console final : public Thread {
  public:
    Ansi<PRINTER> *ansi;
    Parser *parser;

    explicit Console(const ID &id = ID("console")) :
        Thread(id), ansi(new Ansi<PRINTER>(PRINTER::singleton())), parser(new Parser(id)) {}

    void setup() override { Thread::setup(); }

    void loop() override {
      Thread::loop();
      string line = "";
      while (true) {
        this->printPrompt();
        line.clear();
        std::getline(std::cin, line);
        StringHelper::trim(line);
        /////
        if (line.empty()) {
          // do nothing
        } else if (line[0] == ':') {
          if (line == ":quit") {
            this->stop();
            return;
          }
        } else {
          try {
            const ptr<OBJ_OR_BYTECODE> obj = Parser::parseObj(line);
            if (obj->isBytecode()) {
              const ptr<Fluent<>> fluent =  this->parser->parseToFluent(line.c_str());
              this->printResults(fluent);
            } else {
              this->printResult(obj->cast<Obj>());
            }
          } catch (const std::exception &e) {
            // do nothing (log error for now)
            //LOG_EXCEPTION(e);
            this->printException(e);
          }
        }
      }
    }

    void stop() override { Thread::stop(); }

    void printException(const std::exception &ex) const { this->ansi->printf("[!rERROR!!] %s\n", ex.what()); }

    void printPrompt() const { this->ansi->print("!mfhatos!!> "); }

    void printResults(const ptr<Fluent<>> &fluent) const {
      fluent->forEach<Obj>([this](const Obj *obj) { this->printResult(obj); });
    }


    void printResult(const Obj *obj) const {
      if (obj->type() == OType::REC)
        this->printRec((Rec *) obj);
      else
        this->ansi->printf("!g==!!>%s\n", obj->toString().c_str());
    }

    void printRec(const Rec *rec, int i = 0) const {
      this->ansi->printf("!g==!!>!y%s!![", rec->utype() ? rec->utype()->toString().c_str() : "");
      const int size = rec->value()->size();
      for (const auto &[key, value]: *rec->value()) {
        if (i > 0)
          this->ansi->print("    ");
        this->ansi->printf("%s !b=>!! %s", key->toString().c_str(), value->toString().c_str());
        i++;
        if (i < size) {
          this->ansi->print("\n");
        }
      }
      this->ansi->printf("]\n");
    }
  };
} // namespace fhatos

#endif
