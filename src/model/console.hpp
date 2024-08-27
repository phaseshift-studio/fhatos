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
#include <furi.hpp>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <process/actor/actor.hpp>
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)
#include <model/terminal.hpp>
#include <structure/stype/empty.hpp>

namespace fhatos {
  using Command = Trip<string, Consumer<Obj_p>, Runnable>;
  static Map<string, Command> *_MENU_MAP = nullptr;

  class Console final : public Actor<Thread, KeyValue> {
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
      if (obj->is_objs()) {
        for (Obj_p &o: *obj->objs_value()) {
          this->printResult(o, depth + 1);
        }
      } else if (this->_nesting && obj->is_lst()) {
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

  protected:
    explicit Console(const ID &id = ID("/io/repl/")) : Actor<Thread, KeyValue>(id) {
      if (!_MENU_MAP) {
        _MENU_MAP = new Map<string, Command>();
        _MENU_MAP->insert({":help",
                           {"help menu", [](const Obj_p &) { std::get<2>(_MENU_MAP->at(":help"))(); },
                            []() {
                              printer<>()->println("!m!_FhatOS !g!_Console Commands!!");
                              for (const auto &[command, description]: *_MENU_MAP) {
                                printer<>()->printf("!y%-10s!! %s\n", command.c_str(),
                                                    std::get<0>(description).c_str());
                              }
                            }}});
        _MENU_MAP->insert({":log",
                           {"log level",
                            [](const Uri_p &log_level) {
                              Options::singleton()->log_level(LOG_TYPES.toEnum(log_level->uri_value().toString()));
                              return log_level;
                            },
                            [] {
                              printer<>()->printf(
                                      "!ylog!!: !b%s!!\n",
                                      LOG_TYPES.toChars(Options::singleton()->log_level<LOG_TYPE>()).c_str());
                            }}});
        _MENU_MAP->insert({":output",
                           {"terminal output", [](const Obj_p &obj) { Terminal::currentOut(id_p(obj->uri_value())); },
                            [] {
                              printer<>()->printf("!youtput!!: !b%s!! !y=>!! !b%s!!\n",
                                                  Terminal::currentOut()->toString().c_str(),
                                                  Terminal::singleton()->id()->extend("out").toString().c_str());
                            }}});
        /*_MENU_MAP->insert({":router",
                           {"pubsub router",
                            [](const Obj_p &obj) {
                              if (obj->uri_value().matches("/sys/router/global"))
                                Options::singleton()->router<Router>(MqttRouter::singleton());
                              else if (obj->uri_value().matches("/sys/router/local"))
                                Options::singleton()->router<Router>(LocalRouter::singleton());
                              else
                                Options::singleton()->router<Router>(MetaRouter::singleton());
                            },
                            [] {
                              Options::singleton()->printer<>()->printf(
                                  "!yrouter!!: !b%s!!\n",
                                  Options::singleton()->router<Router>()->id()->toString().c_str());
                            }}});*/
        _MENU_MAP->insert({":color",
                           {"colorize output", [this](const Bool_p &xbool) { this->_color = xbool->bool_value(); },
                            [this] { printer<>()->printf("!ycolor!!: %s\n", FOS_BOOL_STR(this->_color)); }}});
        _MENU_MAP->insert(
                {":nesting",
                 {"display poly objs nested", [this](const Bool_p &xbool) { this->_nesting = xbool->bool_value(); },
                  [this] { printer<>()->printf("!ynesting!!: %s\n", FOS_BOOL_STR(this->_nesting)); }}});
        _MENU_MAP->insert({":shutdown",
                           {"kill scheduler", [](const Obj_p &) { Scheduler::singleton()->stop(); },
                            []() {
                              Scheduler::singleton()->stop();
                            }}});
        _MENU_MAP->insert(
                {":quit", {"kill console process", [this](const Obj_p &) { this->stop(); }, [this] { this->stop(); }}});
      }
    }

  public:
    static ptr<Console> create(const ID &id = ID("/io/console")) {
      auto *console = new Console(id);
      return ptr<Console>(console);
    }

    void loop() override {
      Actor::loop();
      //// PROMPT
      if (this->_newInput)
        this->printPrompt(!this->_line.empty());
      this->_newInput = false;
      //// READ CHAR INPUT ONE-BY-ONE
      int x;
      if ((x = Terminal::readChar()) == EOF)
        return;
      // LOG(TRACE, "key pressed: (dec) %i (hex) 0x%x (char) %c\n", x, x, x);
      /*if (0x147 == (char) x) /// CTRL-DELETE (clear line)
        this->_line.clear();
      else if (0x59 == (char) x) /// F1 (toggle logger)
        std::get<1>(_MENU_MAP->at(":log"))(
            Obj::to_str(LOG_TYPES.toChars((LOG_TYPE) (GLOBAL_OPTIONS->logger<uint8_t>() + 1))));
      else*/
      if ('\n' == (char) x)
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
      ///////// PARSE MULTI-LINE MONOIDS
      size_t pos = this->_line.find("###");
      while (pos != string::npos) {
        this->_line.replace(pos, 3, "");
        pos = this->_line.find("###", pos + 0);
      }
      LOG_PROCESS(DEBUG, this, "line to parse: %s\n", this->_line.c_str());
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
            string value = this->_line.substr(index);
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
          this->printResult(Fluent(obj.value()).toObjs());
        } catch (const std::exception &e) {
          this->printException(e);
        }
        this->_line.clear();
      }
    }
  };
} // namespace fhatos

#endif
