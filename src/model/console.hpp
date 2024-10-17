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
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)
#include <model/terminal.hpp>
#include <process/obj_process.hpp>

namespace fhatos {
  using Command = Pair<string, Function<Obj_p, Obj_p>>;
  static Map<string, Command> *MENU_MAP_ = nullptr;

  class Console final : public Thread {
  public:
    ~Console() override { delete MENU_MAP_; }

    struct Settings {
      bool nest = false;
      bool ansi = true;
      bool strict = false;
      LOG_TYPE log = LOG_TYPE::INFO;

      Settings() {
      };

      Settings(const bool nest, const bool ansi, const bool strict, const LOG_TYPE log) {
        this->nest = nest;
        this->ansi = ansi;
        this->strict = strict;
        this->log = log;
      };
    };

  protected:
    string line_;
    ID_p terminal_id_;
    bool new_input_ = true;
    Settings settings_;

    ///// printers
    void print_exception(const std::exception &ex) const {
      router()->write(this->id(), str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())), false);
    }

    void print_prompt(const bool blank = false) const {
      router()->write(this->id(), str(blank ? "        " : "!mfhatos!g>!! "), false);
    }

    void print_result(const Obj_p &obj, const uint8_t depth = 0) const {
      if (obj->is_objs())
        for (Obj_p &o: *obj->objs_value()) {
          this->print_result(o, depth + 1);
        }
      else if (this->settings_.nest && (obj->is_lst() || obj->is_objs())) {
        router()->write(this->id(),
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m" + (
                              obj->is_lst() ? "[" : "{") + "!!\n"), false);
        for (const auto &e: *obj->lst_value()) {
          router()->write(this->id(), str(StringHelper::format(
                            "%s%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                            e->is_poly()
                              ? ""
                              : e->toString(true, true, this->settings_.ansi, this->settings_.strict).c_str())), false);
          if (e->is_poly())
            this->print_result(e, depth + 1);
        }
        router()->write(
          this->id(),
          str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
              (obj->type()->path_length() > 2 ? StringHelper::repeat(obj->type()->name().length(), " ").c_str() : "") +
              "!m" + (obj->is_lst() ? "]" : "}") + "!!\n"), false);
      } else if (this->settings_.nest && obj->is_rec()) {
        router()->write(this->id(),
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m[!!\n"), false);
        for (const auto &[key, value]: *obj->rec_value()) {
          router()->write(this->id(),
                          str(StringHelper::format(
                            "%s!c%s!m=>!!%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                            key->toString(true, false, false, this->settings_.strict).c_str(),
                            value->is_poly()
                              ? ""
                              : value->toString(
                                true, true, this->settings_.ansi,
                                this->settings_.strict).c_str())),
                          false);
          if (value->is_poly())
            this->print_result(value, depth + 1);
        }
        router()->write(
          this->id(),
          str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
              (obj->type()->path_length() > 2 ? StringHelper::repeat(obj->type()->name().length(), " ").c_str() : "") +
              "!m]!!\n"), false);
      } else {
        router()->write(this->id(), str(string("!g") + StringHelper::repeat(depth, "=")), false);
        router()->write(this->id(), str(StringHelper::format("==>!!%s\n",
                                                             obj->toString(
                                                               true, true, this->settings_.ansi, this->settings_.strict).
                                                             c_str())), false);
      }
    }

    explicit Console(const ID &id, const ID &terminal, const Settings &settings) : Thread(id),
      terminal_id_(id_p(terminal)),
      settings_(settings) {
      if (!MENU_MAP_) {
        MENU_MAP_ = new Map<string, Command>();
        MENU_MAP_->insert({":help",
          {"help menu", [](const Obj_p &) {
            printer<>()->println("!m!_FhatOS !g!_Console Commands!!");
            for (const auto &[command, description]: *MENU_MAP_) {
              printer<>()->printf("!y%-10s!! %s\n", command.c_str(),
                                  std::get<0>(description).c_str());
            }
            return noobj();
          }}});
        MENU_MAP_->insert({":log",
          {"log level",
            [](const Uri_p &log_level) {
              if (log_level->is_noobj())
                return vri(LOG_TYPES.to_chars(Options::singleton()->log_level<LOG_TYPE>()));
              Options::singleton()->log_level(LOG_TYPES.to_enum(log_level->uri_value().toString()));
              return log_level;
            }}});
        MENU_MAP_->insert(
          {":output",
            {"terminal out id", [this](const Uri_p &uri) {
              if (!uri->is_noobj())
                this->terminal_id_ = id_p(uri->uri_value());
              return Obj::to_uri(*this->terminal_id_);
            }}});
        MENU_MAP_->insert({":clear",
          {"clear terminal", [](const Obj_p &) {
            printer<>()->print("!X!Q");
            return noobj();
          }}});
        MENU_MAP_->insert({":color",
          {"colorize output", [this](const Bool_p &xbool) {
            if (!xbool->is_noobj())
              this->settings_.ansi = xbool->bool_value();
            return dool(this->settings_.ansi);
          }}});
        MENU_MAP_->insert({":strict",
          {"strict formatting", [this](const Bool_p &xbool) {
            if (!xbool->is_noobj())
              this->settings_.strict = xbool->bool_value();
            return dool(this->settings_.strict);
          }}});
        MENU_MAP_->insert({":nest",
          {"display poly objs nested", [this](const Bool_p &xbool) {
            if (!xbool->is_noobj())
              this->settings_.nest = xbool->bool_value();
            return dool(this->settings_.nest);
          }}});
        MENU_MAP_->insert(
          {":shutdown",
            {
              // TODO: MAKE THIS MAIL CONSTRUCTION A FUNCTION CALL IN SCHEDULER
              "kill scheduler",
              [this](const Obj_p &) {
                Scheduler::singleton()->recv_mail(share(
                  Mail{share(Subscription{
                      .source = fURI(*this->id()),
                      .pattern = *Scheduler::singleton()->id(),
                      .on_recv = Insts::to_bcode([](const Message_p &) { Scheduler::singleton()->stop(); })}),
                    message_p(*Scheduler::singleton()->id(), noobj(), true)}));
                this->delay(250);
                return noobj();
              }}});
      }
    }

  public:
    static ptr<Console> create(const ID &id = ID("/io/console"), const ID &terminal = ID("/terminal/"),
                               const Console::Settings &settings = Settings()) {
      const auto console = ptr<Console>(new Console(id, terminal, settings));
      const Rec_p console_rec = console->to_rec();
      router()->write(id_p(id.resolve("./terminal")), vri(terminal));
      //  router()->write(id_p(id), console_rec);
      return console;
    }

    Rec_p to_rec() {
      // const ID settings_id = this->id()->resolve("./config");
      router()->write(this->id(), load_process(PtrHelper::no_delete<Console>(this), __FILE__, 221, 241));
      router()->write(id_p(this->id()->extend("config/")), Obj::to_rec({
                        {vri(this->id()->extend("config/nest")), dool(this->settings_.nest)},
                        {vri(this->id()->extend("config/strict")), dool(this->settings_.strict)},
                        {vri(this->id()->extend("config/ansi")), dool(this->settings_.ansi)},
                        {vri(this->id()->extend("config/log")), vri(LOG_TYPES.to_chars(this->settings_.log))},
                        {vri(this->id()->extend("config/clear")),
                          OBJ_PARSER("{'!'}.plus('X').plus('!').plus('Q').print(_)")}
                      }));
      return noobj();
    }

    void setup() override {
      Thread::setup();
      router()->write(this->terminal_id_, vri(*this->id()));
      router()->route_subscription(subscription_p(
        *this->id(),
        this->id()->extend("config/+"),
        Insts::to_bcode([this](const Message_p &message) {
          if (message->retain) {
            if (message->target.name() == "nest")
              this->settings_.nest = message->payload->bool_value();
            else if (message->target.name() == "strict")
              this->settings_.strict = message->payload->bool_value();
            else if (message->target.name() == "ansi")
              this->settings_.ansi = message->payload->bool_value();
            else if (message->target.name() == "log")
              this->settings_.log = LOG_TYPES.to_enum(message->payload->uri_value().toString());
          }
        })));
    }

    void loop() override {
      Thread::loop();
      //// PROMPT
      if (this->new_input_)
        this->print_prompt(!this->line_.empty());
      this->new_input_ = false;
      //// READ CHAR INPUT ONE-BY-ONE
      int x;
      if ((x = Terminal::readChar()) == EOF)
        return;
      if ('\n' == static_cast<char>(x)) {
        this->new_input_ = true;
        this->line_ += static_cast<char>(x);
      } else {
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
            this->print_exception(fError("!g[!b%s!g] !b%s!! is an unknown !yconsole command!!",
                                         this->id()->toString().c_str(), command.c_str()));
          } else if (index == string::npos) {
            this->print_result(MENU_MAP_->at(command).second(noobj()));
          } else {
            const string value = this->line_.substr(index);
            StringHelper::trim(value);
            this->print_result(MENU_MAP_->at(command).second(parse(value)->apply(noobj())));
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
            throw fError("Unable to parse input: %s", this->line_.c_str());
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
