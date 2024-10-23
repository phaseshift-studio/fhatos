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

  class Console final : public Thread {
  public:
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

    ///// printers
    void print_exception(const std::exception &ex) const {
      router()->write(this->id(), str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())), false);
    }

    void print_prompt(const bool blank = false) const {
      router()->write(this->id(), str(blank ? "        " : "!mfhatos!g>!! "), false);
    }

    void print_result(const Obj_p &obj, const uint8_t depth = 0) const {
      ///// read configuration
      const bool nest = router()->read(id_p(this->id()->extend("config/nest")))->bool_value();
      const bool ansi = router()->read(id_p(this->id()->extend("config/ansi")))->bool_value();
      const bool strict = router()->read(id_p(this->id()->extend("config/strict")))->bool_value();
      /////
      if (obj->is_objs())
        for (Obj_p &o: *obj->objs_value()) {
          this->print_result(o, depth + 1);
        }
      else if (nest && (obj->is_lst() || obj->is_objs())) {
        router()->write(this->id(),
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m" +
                            (obj->is_lst() ? "[" : "{") + "!!\n"),
                        false);
        for (const auto &e: *obj->lst_value()) {
          router()->write(
            this->id(),
            str(StringHelper::format(
              "%s%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
              e->is_poly() ? "" : e->toString(true, true, ansi, strict).c_str())),
            false);
          if (e->is_poly())
            this->print_result(e, depth + 1);
        }
        router()->write(
          this->id(),
          str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
              (obj->type()->path_length() > 2
                 ? StringHelper::repeat(obj->type()->name().length(), " ").c_str()
                 : "") +
              "!m" + (obj->is_lst() ? "]" : "}") + "!!\n"),
          false);
      } else if (nest && obj->is_rec()) {
        router()->write(this->id(),
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m[!!\n"),
                        false);
        for (const auto &[key, value]: *obj->rec_value()) {
          router()->write(this->id(),
                          str(StringHelper::format(
                            "%s!c%s!m=>!!%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                            key->toString(true, false, false, strict).c_str(),
                            value->is_poly()
                              ? ""
                              : value->toString(true, true, ansi, strict).c_str())),
                          false);
          if (value->is_poly())
            this->print_result(value, depth + 1);
        }
        string obj_string =
            string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
            (obj->type()->path_length() > 2 ? StringHelper::repeat(obj->type()->name().length(), " ").c_str() : "") +
            "!m]";
        if (obj->id()) {
          obj_string += "!m@!b";
          obj_string += obj->id()->toString();
        }
        obj_string += "!!\n";
        router()->write(this->id(), str(obj_string), false);
      } else {
        router()->write(this->id(), str(string("!g") + StringHelper::repeat(depth, "=")), false);
        router()->write(
          this->id(),
          str(StringHelper::format("==>!!%s\n",
                                   obj->toString(true, true, ansi, strict).c_str())),
          false);
      }
    }

    explicit Console(const ID &id, const ID &terminal, const Settings &settings) : Thread(id),
      terminal_id_(id_p(terminal)) {
      router()->write(id_p(id.resolve("./config/nest")), dool(settings.nest));
      router()->write(id_p(id.resolve("./config/nest").query("doc")), str("pretty print polys"));
      router()->write(id_p(id.resolve("./config/strict")), dool(settings.strict));
      router()->write(id_p(id.resolve("./config/strict").query("doc")), str("strict formatting"));
      router()->write(id_p(id.resolve("./config/ansi")), dool(settings.ansi));
      router()->write(id_p(id.resolve("./config/ansi").query("doc")), str("colorize output"));
      router()->write(id_p(id.resolve("./config/log")), vri(LOG_TYPES.to_chars(settings.log)));
      router()->write(id_p(id.resolve("./config/log").query("doc")), str("log level"));
    }

  public:
    static ptr<Console> create(const ID &id = ID("/io/console"), const ID &terminal = ID("/terminal/"),
                               const Console::Settings &settings = Settings()) {
      const auto console = ptr<Console>(new Console(id, terminal, settings));
      const Rec_p console_rec = console->to_rec();
      router()->write(id_p(id.resolve("./terminal")), vri(terminal));
      return console;
    }

    Rec_p to_rec() {
      // const ID settings_id = this->id()->resolve("./config");
      router()->write(this->id(), load_process(PtrHelper::no_delete<Console>(this), __FILE__, 221, 241));
      router()->write(
        id_p(this->id()->extend("config/")),
        Obj::to_rec(
          {{vri(this->id()->extend("config/nest")), router()->read(id_p(this->id()->extend("config/nest")))},
            {vri(this->id()->extend("config/strict")), router()->read(id_p(this->id()->extend("config/strict")))},
            {vri(this->id()->extend("config/ansi")), router()->read(id_p(this->id()->extend("config/ansi")))},
            {vri(this->id()->extend("config/log")), router()->read(id_p(this->id()->extend("config/log")))},
            {vri(this->id()->extend("config/clear")), parse("{'!'}.plus('X').plus('!').plus('Q').print(_)")}}));
      return noobj();
    }

    void setup() override {
      Thread::setup();
      router()->write(this->terminal_id_, vri(*this->id()));

      //////////////////////////////////////////
      router()->route_subscription(subscription_p(
        *this->id(), this->id()->resolve("./config/+"), Insts::to_bcode([this](const Message_p &message) {
          if (message->retain && !message->target.has_query()) {
            if (message->target.name() == "ansi") {
              Options::singleton()->printer<Ansi<>>()->on(message->payload->bool_value());
            } else if (message->target.name() == "log") {
              Options::singleton()->log_level(LOG_TYPES.to_enum(message->payload->uri_value().toString()));
            }
          }
        })));
      router()->route_subscription(subscription_p(
        *this->id(), this->id()->resolve("./prompt"), Insts::to_bcode([this](const Message_p &message) {
          router()->write(this->id_, str(message->payload->str_value() + "\n"), false);
          this->process_line(message->payload->str_value());
          this->print_prompt();
        })));
    }

    void process_line(string line) const {
      LOG_PROCESS(DEBUG, this, "line to parse: %s\n", line.c_str());
      StringHelper::trim(line);

        ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
        try {
          if (line[0] == '\n')
            line = line.substr(1);
          const Option<Obj_p> obj = Parser::singleton()->try_parse_obj(line);
          if (!obj.has_value())
            throw fError("Unable to parse input: %s", line.c_str());
          this->print_result(Options::singleton()->processor<Obj, BCode, Obj>(
            obj.value()->is_bcode() ? noobj() : obj.value(), obj.value()->is_bcode() ? obj.value() : bcode()));
        } catch (const std::exception &e) {
          this->print_exception(e);
        }
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
      this->process_line(this->line_);
      this->line_.clear();
    }
  };
} // namespace fhatos

#endif
