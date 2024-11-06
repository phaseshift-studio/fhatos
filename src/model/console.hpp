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

namespace fhatos {
  class Console final : public Thread {
  public:
    struct Settings {
      bool nest_;
      bool ansi_;
      bool strict_;
      LOG_TYPE log_;

      Settings(const bool nest, const bool ansi, const bool strict, const LOG_TYPE log) :
        nest_(nest), ansi_(ansi),
        strict_(strict), log_(log) {
      };
    };

  protected:
    string line_;
    ID_p stdin_id;
    ID_p stdout_id;
    bool new_input_ = true;

    ///// printers
    void print_exception(const std::exception &ex) const {
      router()->write(this->stdout_id, str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())), false);
    }

    void print_prompt(const bool blank = false) const {
      router()->write(this->stdout_id, str(blank ? "        " : "!mfhatos!g>!! "), false);
    }

    void print_result(const Obj_p &obj, const uint8_t depth = 0) const {
      ///// read configuration
      const bool nest = router()->read(id_p(this->id()->extend("config/nest")))->bool_value();
      const bool ansi = router()->read(id_p(this->id()->extend("config/ansi")))->bool_value();
      const bool strict = router()->read(id_p(this->id()->extend("config/strict")))->bool_value();
      const LOG_TYPE log = LOG_TYPES.to_enum(
          router()->read(id_p(this->id()->extend("config/log")))->uri_value().toString());
      Options::singleton()->log_level(log);

      /////
      if (obj->is_objs())
        for (Obj_p &o: *obj->objs_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          this->print_result(o, depth + 1);
        }
      else if (nest && (obj->is_lst() || obj->is_objs())) {
        router()->write(this->stdout_id,
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m" +
                            (obj->is_lst() ? "[" : "{") + "!!\n"),
                        false);
        for (const auto &e: *obj->lst_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          router()->write(this->stdout_id,
                          str(StringHelper::format(
                              "%s%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                              e->is_poly() ? "" : e->toString(true, true, ansi, strict).c_str())),
                          false);
          if (e->is_poly())
            this->print_result(e, depth + 1);
        }
        router()->write(this->stdout_id,
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2
                               ? StringHelper::repeat(obj->type()->name().length(), " ").c_str()
                               : "") +
                            "!m" + (obj->is_lst() ? "]" : "}") + "!!\n"),
                        false);
      } else if (nest && obj->is_rec()) {
        router()->write(this->stdout_id,
                        str(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                            (obj->type()->path_length() > 2 ? obj->type()->name().c_str() : "") + "!m[!!\n"),
                        false);
        for (const auto &[key, value]: *obj->rec_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          router()->write(this->stdout_id,
                          str(StringHelper::format(
                              "%s!c%s!m=>!!%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
                              key->toString(true, false, false, strict).c_str(),
                              value->is_poly() ? "" : value->toString(true, true, ansi, strict).c_str())),
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
        router()->write(this->stdout_id, str(obj_string), false);
      } else {
        router()->write(this->stdout_id, str(string("!g") + StringHelper::repeat(depth, "=")), false);
        router()->write(this->stdout_id,
                        str(StringHelper::format("==>!!%s\n", obj->toString(true, true, ansi, strict).c_str())), false);
      }
    }

    void process_line(string line) const {
      LOG_PROCESS(DEBUG, this, "line to parse: %s\n", line.c_str());
      StringHelper::trim(line);
      ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
      try {
        if (line[0] == '\n')
          line = line.substr(1);
        if (line.empty()) {
          router()->write(this->stdout_id, str("\n"), false);
          return;
        }
        const Option<Obj_p> obj = Parser::singleton()->try_parse_obj(line);
        if (!obj.has_value())
          throw fError("unable to parse input: %s", line.c_str());
        this->print_result(Options::singleton()->processor<Obj, BCode, Obj>(
            obj.value()->is_bcode() ? noobj() : obj.value(), obj.value()->is_bcode() ? obj.value() : bcode()));
      } catch (const std::exception &e) {
        this->print_exception(e);
      }
    }

    explicit Console(const ID &id, const ID &terminal, const Settings &settings) :
      Thread(rec({{vri(":loop"),
                   Obj::to_bcode([this](const Obj_p &) -> Obj_p {
                     if (this->new_input_)
                       this->print_prompt(!this->line_.empty());
                     this->new_input_ = false;
                     //// READ CHAR INPUT ONE-BY-ONE
                     int x;
                     if ((x = router()->exec(this->stdin_id, noobj())->int_value()) == EOF)
                       return noobj();
                     if ('\n' == static_cast<char>(x)) {
                       this->new_input_ = true;
                       this->line_ += static_cast<char>(x);
                     } else {
                       this->line_ += static_cast<char>(x);
                       return noobj();
                     }
                     StringHelper::trim(this->line_);
                     if (this->line_.empty()) {
                       ///////// DO NOTHING ON EMPTY LINE
                       return noobj();
                     }
                     if (!Parser::closed_expression(this->line_))
                       return noobj();
                     ///////// PARSE MULTI-LINE MONOIDS
                     size_t pos = this->line_.find("###");
                     while (pos != string::npos) {
                       this->line_.replace(pos, 3, "");
                       pos = this->line_.find("###", pos);
                     }
                     this->process_line(this->line_);
                     this->line_.clear();
                     return noobj();
                   }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
                  {vri(":prompt"), Obj::to_bcode([this](const Obj_p &obj) {
                    printer<>()->printf("%s\n", obj->str_value().c_str());
                    this->process_line(obj->str_value());
                    this->print_prompt();
                    return noobj();
                  }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
                  {vri("config"), rec({{vri("nest"), dool(settings.nest_)},
                                       {vri("strict"), dool(settings.strict_)},
                                       {vri("ansi"), dool(settings.ansi_)},
                                       {vri("log"), vri(LOG_TYPES.to_chars(settings.log_))}
                   })}}, THREAD_FURI, id_p(id))),
      stdin_id(id_p(terminal.resolve("./:stdin"))),
      stdout_id(id_p(terminal.resolve("./:stdout"))) {
    }

  public:
    static ptr<Console> create(const ID &id, const ID &terminal, const Console::Settings &settings) {
      const auto console = ptr<Console>(new Console(id, terminal, settings));
      return console;
    }
  };
} // namespace fhatos

#endif