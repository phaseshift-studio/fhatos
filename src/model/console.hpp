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

namespace fhatos {
  using Command = Trip<string, Consumer<Obj_p>, Runnable>;
  static Map<string, Command> *MENU_MAP_ = nullptr;

  class Console final : public Actor<Thread, KeyValue> {
  public:
    ~Console() override {
      delete MENU_MAP_;
    }

  protected:
    string line_;
    bool new_input_ = true;
    bool nest_ = false;
    bool color_ = true;

    ///// printers
    void print_exception(const std::exception &ex) const { Terminal::out(this->id(), "!r[ERROR]!! %s", ex.what()); }

    void print_prompt(const bool blank = false) const {
      Terminal::out(this->id(), blank ? "        " : "!mfhatos!g>!! ");
    }

    void print_result(const Obj_p &obj, const uint8_t depth = 0) const {
      if (obj->is_objs())
        for (Obj_p &o: *obj->objs_value()) {
          this->print_result(o, depth + 1);
        }
      else if (this->nest_ && obj->is_lst()) {
        bool first = true;
        for (const auto &e: *obj->lst_value()) {
          Terminal::out(this->id(), "%s%s%s\n!!",
                        (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                        first ? "!m[!!" : " ",
                        e->is_poly() ? "" : e->toString().c_str());
          if (e->is_poly())
            this->print_result(e, depth + 1);
          first = false;
        }
        Terminal::out(this->id(), (string("!g") + StringHelper::repeat(depth, "=") + ">!m]!!\n").c_str());
      } else if (this->nest_ && obj->is_rec()) {
        bool first = true;
        for (const auto &[key, value]: *obj->rec_value()) {
          Terminal::out(this->id(), "%s%s%s!m=>!!%s\n!!",
                        (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                        first ? "!m[!!" : " ",
                        key->toString().c_str(),
                        value->is_poly() ? "" : value->toString().c_str());
          if (value->is_poly())
            this->print_result(value, depth + 1);
          first = false;
        }
        Terminal::out(this->id(), (string("!g") + StringHelper::repeat(depth, "=") + ">!m]!!\n").c_str());
      } else {
        Terminal::out(this->id(), (string("!g") + StringHelper::repeat(depth, "=")).c_str());
        Terminal::out(this->id(), "==>!!%s\n", obj->toString().c_str());
      }
    }

    explicit Console(const ID &id = ID("/io/repl/")) : Actor<Thread, KeyValue>(id) {
      if (!MENU_MAP_) {
        MENU_MAP_ = new Map<string, Command>();
        MENU_MAP_->insert({
          ":help",
          {
            "help menu", [](const Obj_p &) { std::get<2>(MENU_MAP_->at(":help"))(); },
            []() {
              printer<>()->println("!m!_FhatOS !g!_Console Commands!!");
              for (const auto &[command, description]: *MENU_MAP_) {
                printer<>()->printf("!y%-10s!! %s\n", command.c_str(),
                                    std::get<0>(description).c_str());
              }
            }
          }
        });
        MENU_MAP_->insert({
          ":log",
          {
            "log level",
            [](const Uri_p &log_level) {
              Options::singleton()->log_level(LOG_TYPES.to_enum(log_level->uri_value().toString()));
              return log_level;
            },
            [] {
              printer<>()->printf(
                "!ylog!!: !b%s!!\n",
                LOG_TYPES.to_chars(Options::singleton()->log_level<LOG_TYPE>()).c_str());
            }
          }
        });
        MENU_MAP_->insert({
          ":output",
          {
            "terminal output", [](const Obj_p &obj) { Terminal::currentOut(id_p(obj->uri_value())); },
            [] {
              printer<>()->printf("!youtput!!: !b%s!! !y=>!! !b%s!!\n",
                                  Terminal::currentOut()->toString().c_str(),
                                  Terminal::singleton()->id()->extend("out").toString().c_str());
            }
          }
        });
        MENU_MAP_->insert({
          ":clear", {
            "clear terminal", [](const Obj_p &) {
              printer<>()->print("!X!Q");
            },
            [] { printer<>()->print("!X!Q"); }
          }
        });
        MENU_MAP_->insert({
          ":color",
          {
            "colorize output", [this](const Bool_p &xbool) { this->color_ = xbool->bool_value(); },
            [this] { printer<>()->printf("!ycolor!!: %s\n", FOS_BOOL_STR(this->color_)); }
          }
        });
        MENU_MAP_->insert(
          {
            ":nest",
            {
              "display poly objs nested", [this](const Bool_p &xbool) { this->nest_ = xbool->bool_value(); },
              [this] { printer<>()->printf("!ynest!!: %s\n", FOS_BOOL_STR(this->nest_)); }
            }
          });
        MENU_MAP_->insert({
          ":shutdown",
          {
            // TODO: MAKE THIS MAIL CONSTRUCTION A FUNCTION CALL IN SCHEDULER
            "kill scheduler", [this](const Obj_p &) {
              Scheduler::singleton()->recv_mail(share(Mail{
                share(Subscription{
                  .source = fURI(*this->id()), .pattern = *Scheduler::singleton()->id(),
                  .on_recv = Insts::to_bcode([](const Message_p &) { Scheduler::singleton()->stop(); })
                }),
                share(Message{.target = *Scheduler::singleton()->id(), .payload = noobj(), .retain = true})
              }));
            },
            [this]() {
              Scheduler::singleton()->recv_mail(share(Mail{
                share(Subscription{
                  .source = fURI(*this->id()), .pattern = *Scheduler::singleton()->id(),
                  .on_recv = Insts::to_bcode([](const Message_p &) { Scheduler::singleton()->stop(); })
                }),
                share(Message{.target = *Scheduler::singleton()->id(), .payload = noobj(), .retain = true})
              }));
            }
          }
        });
        MENU_MAP_->insert(
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
      if (this->new_input_)
        this->print_prompt(!this->line_.empty());
      this->new_input_ = false;
      //// READ CHAR INPUT ONE-BY-ONE
      int x;
      if ((x = Terminal::readChar()) == EOF)
        return;
      if ('\n' == static_cast<char>(x))
        this->new_input_ = true;
      else {
        this->line_ += static_cast<char>(x);
        return;
      }
      StringHelper::trim(this->line_);
      if (this->line_.empty()) {
        ///////// DO NOTHING ON EMPTY LINE
        return;
      }
      if (!Parser::closed_expression(this->line_))
        return;
      ///////// PARSE MULTI-LINE MONOIDS
      size_t pos = this->line_.find("###");
      while (pos != string::npos) {
        this->line_.replace(pos, 3, "");
        pos = this->line_.find("###", pos);
      }
      LOG_PROCESS(DEBUG, this, "line to parse: %s\n", this->line_.c_str());
      StringHelper::trim(this->line_);
      if (this->line_[0] == ':') {
        ///////// PARSE MENU COMMANDS
        try {
          const string::size_type index = line_.find_first_of(' ');
          const string command = index == string::npos ? this->line_ : this->line_.substr(0, index);
          StringHelper::trim(command);
          if (!MENU_MAP_->count(command)) {
            this->print_exception(fError("!g[!b%s!g] !b%s!! is an unknown !yconsole command!!\n",
                                         this->id()->toString().c_str(), command.c_str()));
          } else if (index == string::npos) {
            std::get<2>(MENU_MAP_->at(command))();
          } else {
            const string value = this->line_.substr(index);
            StringHelper::trim(value);
            std::get<1>(MENU_MAP_->at(command))(parse(value)->apply(Obj::to_noobj()));
          }
        } catch (std::exception &e) {
          this->print_exception(e);
        }
        this->line_.clear();
      } else {
        ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
        try {
          if (this->line_[0] == '\n')
            this->line_ = this->line_.substr(1);
          const Option<Obj_p> obj = Parser::singleton()->try_parse_obj(this->line_);
          if (!obj.has_value())
            throw fError("Unable to parse input: %s\n", this->line_.c_str());
          this->print_result(Options::singleton()->processor<Obj, BCode, Obj>(
            obj.value()->is_bcode() ? noobj() : obj.value(), obj.value()->is_bcode() ? obj.value() : bcode()));
        } catch (const std::exception &e) {
          this->print_exception(e);
        }
        this->line_.clear();
      }
    }
  };
} // namespace fhatos

#endif
