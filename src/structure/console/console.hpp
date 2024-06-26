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
  template<typename PRINTER = FOS_DEFAULT_PRINTER>
  class Console final : public Thread {
  public:
    explicit Console(const ID &id = ID("console")) :
        Thread(id) {}

    void setup() override { Thread::setup(); }

    void loop() override {
      Thread::loop();
      string line;
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
          } else if (line.starts_with(":log ")) {
            string level = line.substr(5);
            if (!STR_LOGTYPE.count(level)) {
              this->printException(fError("A valid log level required (NONE,DEBUG,INFO,ERROR): %s\n", level.c_str()));
            } else {
              LOGGING_LEVEL = STR_LOGTYPE.at(level);
            }
          }
        } else {
          try {
            const Obj_p obj = Parser::tryParseObj(line).value();
            if (obj->isBytecode()) {
              const Fluent<> fluent = Fluent<>(obj->as("/bcode"));
              this->printResults(fluent);
            } else {
              this->printResult(obj);
            }
          } catch (const std::exception &e) {
            // do nothing (log error for now)
            // LOG_EXCEPTION(e);
            this->printException(e);
          }
        }
      }
    }
    void stop() override { Thread::stop(); }
    ///// printers
    void printException(const std::exception &ex) const { PRINTER::singleton()->printf("!r[ERROR]!! %s", ex.what()); }
    void printPrompt() const { PRINTER::singleton()->print("!mfhatos!!> "); }
    void printResults(const Fluent<> &fluent) const {
      fluent.forEach<Obj>([this](const Obj_p obj) { this->printResult(obj); });
    }
    void printResult(const Obj_p &obj) const { PRINTER::singleton()->printf("!g==>!!%s\n", obj->toString().c_str()); }
  };
} // namespace fhatos

#endif
