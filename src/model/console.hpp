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
#include <util/string_helper.hpp>

#include "language/mmadt/parser.hpp"

#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  class Console final : public Thread {
  public:
    struct Settings {
      uint8_t nest_;
      bool ansi_;
      string prompt_;
      bool strict_;
      LOG_TYPE log_;

      Settings(const uint8_t nest, const bool ansi, const string &prompt, const bool strict,
               const LOG_TYPE log) : nest_(nest), ansi_(ansi), prompt_(prompt),
                                     strict_(strict), log_(log) {
      }
    };

  protected:
    string line_;
    bool new_input_ = true;
    ID_p stdin_id;
    ID_p stdout_id;
    bool direct_stdin_out = true;
    Settings settings_;
    mmadt::Tracker tracker_;
    ///// printers
    void write_stdout(const Str_p &s) const {
      if(this->direct_stdin_out)
        Terminal::STD_OUT_DIRECT(s);
      else
        ROUTER_WRITE(this->stdout_id, s, TRANSIENT);
    }

    Int_p read_stdin() const {
      return this->direct_stdin_out ? Terminal::STD_IN_DIRECT() : router()->exec(this->stdin_id, noobj());
    }

    void print_exception(const std::exception &ex) const {
      this->write_stdout(str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())));
    }

    void print_prompt(const bool blank = false) const {
      const string prompt = this->settings_.prompt_;
      this->write_stdout(str(blank ? StringHelper::repeat(Ansi<>::singleton()->strip(prompt).length()) : prompt));
    }

    void print_result(const Obj_p &obj, const uint8_t depth, string *to_out) const {
      LOG_PROCESS(TRACE, this, "printing processor result: %s\n", obj->toString().c_str());
      if(obj->is_objs())
        for(Obj_p &o: *obj->objs_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          this->print_result(o, depth + 1, to_out);
        }
      else if(this->settings_.nest_ > depth && obj->is_lst()) {
        to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                       (obj->tid()->path_length() > 2 ? obj->tid()->name().c_str() : "") + "!m" +
                       (obj->is_lst() ? "[" : "{") + "!!\n");
        for(const auto &e: *obj->lst_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          to_out->append(StringHelper::format(
            "%s%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
            e->is_poly() ? "" : e->toString().c_str()));
          if(e->is_poly())
            this->print_result(e, depth + 1, to_out);
        }
        to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                       (obj->tid()->path_length() > 2
                          ? StringHelper::repeat(obj->tid()->name().length(), " ").c_str()
                          : "") +
                       "!m" + (obj->is_lst() ? "]" : "}") + "!!\n");
      } else if(this->settings_.nest_ > depth && obj->is_rec()) {
        to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                       (obj->tid()->path_length() > 2 ? obj->tid()->name().c_str() : "") + "!m[!!\n");
        for(const auto &[key, value]: *obj->rec_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          to_out->append(StringHelper::format(
            "%s!c%s!m=>!!%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
            key->toString().c_str(),
            value->is_poly()
              ? ""
              : value->toString().c_str()));
          if(value->is_poly())
            this->print_result(value, depth + 1, to_out);
        }
        string obj_string =
            string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
            //(obj->tid()->path_length() > 2 ? StringHelper::repeat(obj->tid()->name().length(), " ").c_str() : "") +
            "!m]";
        if(obj->vid()) {
          obj_string += "!m@!b";
          obj_string += obj->vid()->toString();
        }
        obj_string += "!!\n";
        to_out->append(obj_string);
      } else {
        to_out->append(string("!g") + StringHelper::repeat(depth, "="));
        to_out->append(StringHelper::format("==>!!%s\n",
                                            obj->toString().c_str()));
      }
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
      LOG_PROCESS(DEBUG, this, "line to parse: %s\n", line.c_str());
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
        LOG_PROCESS(TRACE, this, "processing: %s\n", obj->toString().c_str());
        string to_out;
        this->print_result(obj->is_bcode() ? BCODE_PROCESSOR(obj) : obj, 0, &to_out);
        this->write_stdout(str(to_out));
      } catch(const std::exception &e) {
        this->print_exception(e);
      }
    }

    bool first = true;

    explicit Console(const ID &value_id, const ID &terminal, const Settings &settings) : Thread(Obj::to_rec(rmap({
          {
            "loop", InstBuilder::build(value_id.extend(":loop"))
            ->itype_and_seed(IType::ZERO_TO_ZERO)
            ->inst_f([this](const Obj_p &, const InstArgs &) -> Obj_p {
              if(this->first) {
                this->first = false;
                this->delay(500);
              }
              if(FOS_IS_DOC_BUILD)
                return noobj();
              if(this->new_input_)
                this->print_prompt(!this->line_.empty());
              this->new_input_ = false;
              //// READ CHAR INPUT ONE-BY-ONE
              int x;
              if((x = this->tracker_.track(this->read_stdin()->int_value())) == EOF)
                return noobj();
              if('\n' == static_cast<char>(x)) {
                this->new_input_ = true;
                this->line_ += static_cast<char>(x);
              } else {
                this->line_ += static_cast<char>(x);
                return noobj();
              }
              StringHelper::trim(this->line_);
              if(this->line_.empty() || !this->tracker_.closed()) {
                ///////// DO NOTHING ON EMPTY LINE
                ///////// /r/n ON OPEN EXPRESSION (i.e. multi-line expressions)
                return noobj();
              }
              this->tracker_.clear();
              StringHelper::trim(this->line_);
              this->process_line(this->line_);
              this->line_.clear();
              return noobj();
            })
            ->create()},
          {":prompt", InstBuilder::build(value_id.extend(":prompt"))
            ->domain_range(STR_FURI, NOOBJ_FURI)
            ->itype_and_seed(IType::ONE_TO_ZERO)
            ->type_args(x(0, "code", Obj::to_type(STR_FURI)))
            ->inst_f([this](const Obj_p &, const InstArgs &args) {
              this->print_prompt();
              Terminal::STD_OUT_DIRECT(str(StringHelper::format("%s\n", args->arg(0)->str_value().c_str())));
              string code = args->arg(0)->str_value();
              if(FOS_IS_DOC_BUILD)
                StringHelper::replace(&code, "\\|", "|");
              this->process_line(code);
              return noobj();
            })
            ->create()},
          {"config", rec({{vri("nest"), jnt(settings.nest_)},
            {vri("strict"), dool(settings.strict_)},
            {vri("ansi"), dool(settings.ansi_)},
            {vri("prompt"), str(settings.prompt_)},
            {vri("log"), vri(LOG_TYPES.to_chars(settings.log_))}
          })}
          /*{"terminal", rec({
               {vri("stdin"), vri(terminal.extend(":stdin"))},
               {vri("stdout"), vri(terminal.extend(":stdout"))}})}*/}), THREAD_FURI, id_p(value_id))),
      stdin_id(id_p(terminal.extend(":stdin"))), stdout_id(id_p(terminal.extend(":stdout"))), settings_(settings) {
      /*ROUTER_SUBSCRIBE(Subscription::create(*this->vid_, this->vid_->extend("config/#"),
                                            InstBuilder::build(StringHelper::cxx_f_metadata(__FILE__,__LINE__))->inst_f(
                                              [this](const Obj_p &lhs, const InstArgs &args) {
                                                const string name = Message(lhs).target().name();
                                                if(name == "ansi")
                                                  this->settings_.ansi_ = lhs->bool_value();
                                                else if(name == "nest")
                                                  this->settings_.nest_ = lhs->int_value();
                                                else if(name == "strict")
                                                  this->settings_.strict_ = lhs->bool_value();
                                                else if(name == "prompt")
                                                  this->settings_.prompt_ = lhs->str_value();
                                                else if(name == "log") {
                                                  this->settings_.log_ = LOG_TYPES.to_enum(
                                                    lhs->uri_value().toString());
                                                  Options::singleton()->log_level(this->settings_.log_);
                                                }
                                                return lhs;
                                              })->type_args({x(0)})->create()));*/
    }

  public:
    static ptr<Console> create(const ID &id, const ID &terminal, const Console::Settings &settings) {
      const auto console = ptr<Console>(new Console(id, terminal, settings));
      return console;
    }

    static void *import(const ID &id = "/io/lib/console") {
      // Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      TYPE_SAVER(id_p(id), rec({{vri(":create"),
                                 InstBuilder::build(ID(id.extend(":create")))
                                 ->type_args(
                                   x(0, "install_location", vri(id)),
                                   x(1, "terminal_id", vri(Terminal::singleton()->vid())),
                                   x(2, "config", Obj::to_rec({{vri("nest"), jnt(2)},
                                       {vri("strict"), dool(false)},
                                       {vri("ansi"), dool(true)},
                                       {vri("prompt"), str(">")},
                                       {vri("log"), vri("INFO")}})))
                                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                                   ptr<Console> console = Console::create(
                                     ID(args->arg(0)->uri_value()),
                                     ID(args->arg(1)->uri_value()),
                                     Settings(2, true, "!mfhatos!g>!! ", false, INFO));
                                   return console;
                                 })
                                 ->create()}}, THREAD_FURI));
      return nullptr;
    }
  };
} // namespace fhatos

#endif
