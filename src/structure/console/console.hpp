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
#ifndef fhatos_console_hpp
#define fhatos_console_hpp

#include <fhatos.hpp>
#include <iostream>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <structure/furi.hpp>
#include <util/ansi.hpp>
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)
#include <process/router/local_router.hpp>
#include FOS_MQTT(mqtt_router.hpp)
#include <structure/io/terminal.hpp>

#include <process/actor/actor.hpp>

namespace fhatos {
  class Console final : public Actor<Thread> {
  protected:
    Map<string, Pair<Consumer<Obj_p>, Runnable>> MENU_MAP = Map<string, Pair<Consumer<Obj_p>, Runnable>>();
    ///// printers
    void printException(const std::exception &ex) const { Terminal::out(*this->id(), "!r[ERROR]!! %s", ex.what()); }
    void printPrompt() const { Terminal::out(*this->id(), "!mfhatos!!> "); }
    void printResults(const Fluent &fluent) const {
      fluent.forEach<Obj>([this](const Obj_p &obj) {
        if (obj->isLst()) {
          for (const auto &o: *obj->lst_value()) {
            this->printResult(o);
          }
        } else
          this->printResult(obj);
      });
    }
    void printResult(const Obj_p &obj) const {
      const string output = obj->toString(); //.c_str();// ->c_str();
      Terminal::out(*this->id(), "!g==>!!%s\n", output.c_str());
    }

  public:
    explicit Console(const ID &id = ID("/io/repl/")) : Actor<Thread>(id) {
      MENU_MAP[":quit"] = {[this](const Obj_p &) { this->stop(); }, [this] { this->stop(); }};
      MENU_MAP[":output"] = {[this](const Obj_p &obj) { Terminal::currentOut(share(ID(obj->uri_value()))); },
                             [] {
                               GLOBAL_OPTIONS->printer<>()->printf(
                                   "!youtput!!: !b%s!! !y=>!! !b%s!!\n", Terminal::currentOut()->toString().c_str(),
                                   Terminal::singleton()->id()->extend("out").toString().c_str());
                             }};
    }

    void loop() override {
      Actor<Thread>::loop();
      this->printPrompt();
      string line;
      std::getline(std::cin, line);
      StringHelper::trim(line);
      if (line.empty()) {
        ///////// DO NOTHING ON EMPTY LINE
      } else if (line[0] == ':') {
        ///////// HANDLE MENU INTERACTIONS
        const string::size_type index = line.find_first_of(' ');
        const string command = index == string::npos ? line : line.substr(0, index);
        StringHelper::trim(command);
        if (!MENU_MAP.count(command)) {
          this->printException(fError("Unknown console command: %s\n", command.c_str()));
        } else if (index == string::npos) {
          MENU_MAP[command].second();
        } else {
          const string value = line.substr(index);
          StringHelper::trim(value);
          MENU_MAP[command].first(Parser::singleton()->tryParseObj(value).value()->apply(Obj::to_noobj()));
        }
      } else {
        ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
        try {
          const Option<Obj_p> obj = Parser::singleton()->tryParseObj(line);
          if (obj.value()->isBytecode())
            this->printResults(Fluent(obj.value()));
          else
            this->printResult(obj.value());
        } catch (const std::exception &e) {
          this->printException(e);
        }
      }
    }
  };
} // namespace fhatos

#endif
