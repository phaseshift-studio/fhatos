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
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <structure/furi.hpp>
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)
#include <process/actor/actor.hpp>
#include <structure/io/terminal.hpp>

namespace fhatos {
  using Command = Trip<string, Consumer<Obj_p>, Runnable>;
  static Map<string, Command> *_MENU_MAP = nullptr;
  class Console final : public Actor<Thread> {
  protected:
    string _line;
    bool _newInput = true;
    bool _nesting = false;
    bool _color = true;

    ///// printers
    void printException(const std::exception &ex) const { Terminal::out(*this->id(), "!r[ERROR]!! %s", ex.what()); }
    void printPrompt(const bool blank = false) const {
      Terminal::out(*this->id(), blank ? "        " : "!mfhatos!g>!! ");
    }
    void printResult(const Obj_p &obj, const uint8_t depth = 0) const {
      if (obj->isObjs()) {
        for (Obj_p &o: *obj->objs_value()) {
          this->printResult(o, depth + 1);
        }
      } else if (this->_nesting && obj->isLst()) {
        for (Obj_p &o: *obj->lst_value()) {
          this->printResult(o, depth + 1);
        }
      } else {
        for (uint8_t i = 1; i < depth; i++) {
          Terminal::out(*this->id(), "!g=!!");
        }
        Terminal::out(*this->id(), "!g==>!!%s\n", obj->toString().c_str());
      }
    }

  public:
    explicit Console(const ID &id = ID("/io/repl/")) : Actor<Thread>(id) {
      if (!_MENU_MAP) {
        _MENU_MAP = new Map<string, Command>();
        _MENU_MAP->insert({":help",
                           {"help menu", [this](const Obj_p &) { std::get<2>(_MENU_MAP->at(":help"))(); },
                            [this]() {
                              Terminal::printer<>()->println("!m!_FhatOS !g!_Console Commands!!");
                              for (const auto &pair: *_MENU_MAP) {
                                Terminal::printer<>()->printf("!y%-10s!! %s\n", pair.first.c_str(),
                                                              std::get<0>(pair.second).c_str());
                              }
                            }}});
        _MENU_MAP->insert({":log",
                           {"log level",
                            [this](const Uri_p &logType) {
                              GLOBAL_OPTIONS->LOGGING = LOG_TYPES.toEnum(logType->uri_value().toString().c_str());
                              return logType;
                            },
                            [] {
                              Terminal::printer<>()->printf("!ylog!!: !b%s!!\n",
                                                            LOG_TYPES.toChars(GLOBAL_OPTIONS->logger<LOG_TYPE>()));
                            }}});
        _MENU_MAP->insert(
            {":output",
             {"terminal output", [this](const Obj_p &obj) { Terminal::currentOut(share(ID(obj->uri_value()))); },
              [] {
                Terminal::printer<>()->printf("!youtput!!: !b%s!! !y=>!! !b%s!!\n",
                                              Terminal::currentOut()->toString().c_str(),
                                              Terminal::singleton()->id()->extend("out").toString().c_str());
              }}});
        _MENU_MAP->insert({":color",
                           {"colorize output", [this](const Bool_p &xbool) { this->_color = xbool->bool_value(); },
                            [this] { Terminal::printer<>()->printf("!ycolor!!: %s\n", FOS_BOOL_STR(this->_color)); }}});
        _MENU_MAP->insert(
            {":nesting",
             {"display poly objs nested", [this](const Bool_p &xbool) { this->_nesting = xbool->bool_value(); },
              [this] { Terminal::printer<>()->printf("!ynesting!!: %s\n", FOS_BOOL_STR(this->_nesting)); }}});
        _MENU_MAP->insert({":shutdown",
                           {"destroy scheduler", [this](const Obj_p &) { Scheduler::singleton()->stop(); },
                            [this]() { Scheduler::singleton()->stop(); }}});
        _MENU_MAP->insert(
            {":quit", {"destroy console process", [this](const Obj_p &) { this->stop(); }, [this] { this->stop(); }}});
      }
    }

    void loop() override {
      Actor<Thread>::loop();
      //// PROMPT
      if (this->_newInput)
        this->printPrompt(!this->_line.empty());
      this->_newInput = false;
      //// READ CHAR INPUT ONE-BY-ONE
      int x;
      if ((x = Terminal::readChar()) == EOF)
        return;
      if ('\x04' == (char) x) /// CNTRL-D (clear line)
        this->_line.clear();
      else if ('\n' == (char) x)
        this->_newInput = true;
      else {
        this->_line += (char) x;
        return;
      }
      StringHelper::trim(this->_line);
      if (this->_line.empty()) {
        ///////// DO NOTHING ON EMPTY LINE
        return;
      }
      if (!Parser::closedExpression(this->_line))
        return;
      string replace_word = "###";
      string replace_by = "";
      size_t pos = this->_line.find(replace_word);
      while (pos != string::npos) {
        this->_line.replace(pos, replace_word.size(), replace_by);
        pos = this->_line.find(replace_word, pos + replace_by.size());
      }
      LOG_TASK(DEBUG, this, "line to parse: %s\n", this->_line.c_str());
      StringHelper::trim(this->_line);
      if (this->_line[0] == ':') {
        ///////// PARSE MENU COMMANDS
        try {
          const string::size_type index = _line.find_first_of(' ');
          const string command = index == string::npos ? this->_line : this->_line.substr(0, index);
          StringHelper::trim(command);
          if (!_MENU_MAP->count(command)) {
            this->printException(fError("!g[!b%s!g] !b%s!! is an unknown !yconsole command!!\n",
                                        this->id()->toString().c_str(), command.c_str()));
          } else if (index == string::npos) {
            std::get<2>(_MENU_MAP->at(command))();
          } else {
            const string value = this->_line.substr(index);
            StringHelper::trim(value);
            std::get<1>(_MENU_MAP->at(command))(
                Parser::singleton()->tryParseObj(value).value()->apply(Obj::to_noobj()));
          }
        } catch (std::exception &e) {
          this->printException(e);
        }
        this->_line.clear();
      } else {
        ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
        try {
          const Option<Obj_p> obj = Parser::singleton()->tryParseObj(this->_line);
          if (!obj.has_value())
            throw fError("Unable to parse input: %s\n", this->_line.c_str());
          this->printResult(obj.value()->isBytecode() ? Fluent(obj.value()).toObjs() : obj.value());
        } catch (const std::exception &e) {
          this->printException(e);
        }
        this->_line.clear();
      }
    }
  };
} // namespace fhatos

#endif
