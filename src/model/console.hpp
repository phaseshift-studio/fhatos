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

#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../util/string_helper.hpp"
#include "../lang/mmadt/parser.hpp"
#include "terminal.hpp"
#include "../util/print_helper.hpp"

#include  STR(../process/ptype/HARDWARE/thread.hpp)

namespace fhatos {
  class Console final : public Thread {
  protected:
    string line_;
    bool new_input_ = true;
    /*ID_p stdin_id;
    ID_p stdout_id;*/
    bool direct_stdin_out = true;
    mmadt::Tracker tracker_;
    ///// printers
    void write_stdout(const Str_p &s) const {
      if(this->direct_stdin_out)
        Terminal::STD_OUT_DIRECT(s);
      else
        Router::singleton()->write(this->this_get("config/terminal/stdout")->uri_value(), s, TRANSIENT);
    }

    Str_p read_stdin(const char until) const {
      return this->direct_stdin_out
               ? Terminal::STD_IN_LINE_DIRECT(until)
               : Router::singleton()->exec(this->this_get("config/terminal/stdin")->uri_value(), noobj());
    }

    void print_exception(const std::exception &ex) const {
      this->write_stdout(str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())));
      Router::singleton()->log_frame_stack(ERROR);
    }

    void print_prompt(const bool blank = false) const {
      const string prompt = this->this_get("config/prompt")->str_value();
      this->write_stdout(str(blank ? StringHelper::repeat(Ansi<>::singleton()->strip(prompt).length()) : prompt));
    }

    void process_line(string line) const {
      /////////////////////////////////////////////////////////////
      ////////////// EXPERIMENTING WITH ANSI MOVEMENT /////////////
      if(line == "TEST") {
        this->write_stdout(str("\n\n\n!^u2"));
        this->write_stdout(str("!^S1!y1 3 5 7 9!^L1!^d1!g2 4 6 8!!\n"));
        return;
      }
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      LOG_OBJ(DEBUG, this, "line to parse: %s\n", line.c_str());
      StringHelper::trim(line);
      ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
      try {
        if(line[0] == '\n')
          line = line.substr(1);
        if(line.empty()) {
          this->write_stdout(str("\n"));
          return;
        }
        const Obj_p obj = OBJ_PARSER(line);
        std::stringbuf to_out;
        PrintHelper::pretty_print_obj(BCODE_PROCESSOR(obj),
                                      0,
                                      this->this_get("config/nest")->int_value(),
                                      false,
                                      &to_out);
        this->write_stdout(str(to_out.str()));
      } catch(std::exception &e) {
        this->print_exception(e);
      }
    }

    explicit Console(const ID &value_id, const Rec_p &config) : /* */
      Thread(Obj::to_rec({
                             {":loop", InstBuilder::build(id_p(value_id.extend(":loop")))
                              ->domain_range(NOOBJ_FURI, {0, 0}, NOOBJ_FURI, {0, 0})
                              ->inst_f([this](const Obj_p &, const InstArgs &) -> Obj_p {
                                try {
                                  static bool first = true;
                                  if(first) {
                                    first = false;
                                    this->delay(500);
                                  }
                                  if(FOS_IS_DOC_BUILD)
                                    return noobj();
                                  if(this->new_input_)
                                    this->print_prompt(!this->line_.empty());
                                  this->new_input_ = false;
                                  //// READ CHAR INPUT ONE-BY-ONE
                                  const string x = this->read_stdin('\n')->str_value();
                                  this->tracker_.track(x);
                                  if(this->tracker_.closed()) {
                                    this->new_input_ = true;
                                    this->line_ += x;
                                  } else {
                                    this->line_ += x;
                                    return Obj::to_noobj();
                                  }
                                  StringHelper::trim(this->line_);
                                  if(this->line_.empty() ||
                                     this->line_[this->line_.length() - 1] == ';' ||
                                     // specific to end-step and imperative simulation
                                     !this->tracker_.closed()) {
                                    ///////// DO NOTHING ON OPEN EXPRESSION (i.e. multi-line expressions)
                                    return noobj();
                                  }
                                  // prepare the user input for processing
                                  this->tracker_.clear();
                                  StringHelper::trim(this->line_);
                                  this->process_line(this->line_);
                                  this->line_.clear();
                                } catch(std::exception &e) {
                                  this->print_exception(e);
                                  this->line_.clear();
                                  this->new_input_ = true;
                                }
                                return Obj::to_noobj();
                              })
                              ->create()},
                             {":prompt", InstBuilder::build(value_id.extend(":prompt"))
                              ->domain_range(STR_FURI, {1, 1}, NOOBJ_FURI, {0, 0})
                              ->type_args(x(0, "code", Obj::to_type(STR_FURI)))
                              ->inst_f([this](const Obj_p &, const InstArgs &args) {
                                this->print_prompt();
                                Terminal::STD_OUT_DIRECT(
                                    str(StringHelper::format("%s\n", args->arg(0)->str_value().c_str())));
                                string code = args->arg(0)->str_value();
                                if(STR(BUILD_DOCS) == "ON")
                                  StringHelper::replace(
                                      &code, "\\|", "|");
                                this->process_line(code);
                                return noobj();
                              })
                              ->create()},
                             {"config", config}}, REC_FURI, id_p(value_id))) {
    }

  public:
    static ptr<Console> create(const ID &id, const Rec_p &config) {
      const auto console = ptr<Console>(new Console(id, config));
      return console;
    }

    static void *import() {
      // Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      /*InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->type_args(
            x(0, "install_location", Obj::to_uri(lib_id)),
            x(1, "config", Obj::to_rec({
                {"terminal",
                  Obj::to_rec({
                    {"stdin", vri(Terminal::singleton()->vid->extend(":stdin"))},
                    {"stdout", vri(Terminal::singleton()->vid->extend(":stdout"))}})},
                {"nest", jnt(2)},
                {"strict", dool(false)},
                {"ansi", dool(true)},
                {"prompt", str(">")},
                {"log", vri("INFO")}})))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            ptr<Console> console = Console::create(
              ID(args->arg(0)->uri_value()),
              args->arg(1));
            return console;
          })
          ->save();*/
      return nullptr;
    }
  };
} // namespace fhatos

#endif
