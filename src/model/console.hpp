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
        ROUTER_WRITE(this->this_get("config/terminal/stdout")->uri_p_value<ID>(), s, TRANSIENT);
    }

    Int_p read_stdin() const {
      return this->direct_stdin_out
               ? Terminal::STD_IN_DIRECT()
               : router()->exec(this->this_get("config/terminal/stdin")->uri_p_value<ID>(), noobj());
    }

    void print_exception(const std::exception &ex) const {
      this->write_stdout(str(StringHelper::format("!r[ERROR]!! %s\n", ex.what())));
    }

    void print_prompt(const bool blank = false) const {
      const string prompt = this->this_get("config/prompt")->str_value();
      this->write_stdout(str(blank ? StringHelper::repeat(Ansi<>::singleton()->strip(prompt).length()) : prompt));
    }

    void print_result(const Obj_p &obj, const uint8_t depth, string *to_out, const bool parent_rec = false) const {
      LOG_PROCESS(TRACE, this, "printing processor result: %s\n", obj->toString().c_str());
      const int nest_value = this->this_get("config/nest")->int_value();
      if(obj->is_objs()) {
        //////////////////////// OBJS ///////////////////////////
        for(Obj_p &o: *obj->objs_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          this->print_result(o, depth, to_out);
        }
      } else if(obj->is_lst() && nest_value > depth) {
        //////////////////////// LST ///////////////////////////
        to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                       (obj->tid()->path_length() > 2 ? obj->tid()->name().append("!g[").c_str() : "") + "!m[!!\n");
        for(const auto &e: *obj->lst_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          if(!e->is_poly()) {
            to_out->append(StringHelper::format(
              "%s%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(), e->toString().c_str()));
          } else {
            //to_out->append("!m,!!");
            this->print_result(e, depth + 1, to_out, false);
          }
        }
        to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                       (obj->tid()->path_length() > 2 ? "!g]" : "") + "!m]!!");
        if(obj->vid())
          to_out->append("!m@!b").append(obj->vid()->toString());
        to_out->append("!!\n");
      } else if(obj->is_rec() && nest_value > depth) {
        //////////////////////// REC ///////////////////////////
        if(!parent_rec) {
          to_out->append(string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
                         (obj->tid()->path_length() > 2 ? obj->tid()->name().append("!g[").c_str() : ""));
        }
        to_out->append("!m[!!\n");
        for(const auto &[key, value]: *obj->rec_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          if(!value->is_poly()) {
            to_out->append(StringHelper::format(
              "%s!c%s!m=>!!%s!!\n", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
              key->toString().c_str(),
              value->toString().c_str()));
          } else {
            to_out->append(StringHelper::format(
              "%s!c%s!m=>!!", (string("!g") + StringHelper::repeat(depth, "=") + "==>!!").c_str(),
              key->toString().c_str()));
            this->print_result(value, depth + 1, to_out, true);
          }
        }
        string obj_string =
            string("!g") + StringHelper::repeat(depth, "=") + ">!b" +
            (obj->tid()->path_length() > 2 ? "!g]" : "") +
            "!m]";
        if(obj->vid()) {
          obj_string += "!m@!b";
          obj_string += obj->vid()->toString();
        }
        obj_string += "!!\n";
        to_out->append(obj_string);
      } else {
        //////////////////////// ALL OTHER OBJS ///////////////////////////
        if(parent_rec)
          to_out->append(obj->toString().c_str()).append("\n");
        else {
          to_out->append(string("!g") + StringHelper::repeat(depth, "="));
          to_out->append(StringHelper::format("==>!!%s\n",
                                              obj->toString().c_str()));
        }
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
      } catch(std::exception &e) {
        this->print_exception(e);
      }
    }

    bool first = true;

    explicit Console(const ID &value_id, const Rec_p &config) : Thread(Obj::to_rec(rmap({
        {"loop", InstBuilder::build(
            value_id.extend(":loop"))
          ->itype_and_seed(IType::ZERO_TO_ZERO)
          ->inst_f([this](
          const Obj_p &, const InstArgs &) -> Obj_p {
              try {
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
                if((x = this->tracker_.track(
                      this->read_stdin()->int_value())) ==
                   EOF)
                  return noobj();
                if('\n' == static_cast<char>(x) || '\r' == static_cast<char>(x)) {
                  this->new_input_ = true;
                  this->line_ += static_cast<char>(x);
                } else {
                  this->line_ += static_cast<char>(x);
                  return noobj();
                }
                StringHelper::trim(this->line_);
                if(this->line_.empty() ||
                   this->line_[this->line_.length() - 1] ==
                   ';' ||
                   // specific to end-step and imperative simulation
                   !this->tracker_.closed()) {
                  ///////// DO NOTHING ON OPEN EXPRESSION (i.e. multi-line expressions)
                  return noobj();
                }
                this->tracker_.clear();
                StringHelper::trim(this->line_);
                this->process_line(this->line_);
                this->line_.clear();
              } catch(std::exception &e) {
                this->print_exception(e);
                this->line_.clear();
                this->new_input_ = true;
              }
              return noobj();
            })
          ->create()},
        {":prompt", InstBuilder::build(
            value_id.extend(":prompt"))
          ->domain_range(STR_FURI, NOOBJ_FURI)
          ->itype_and_seed(IType::ONE_TO_ZERO)
          ->type_args(
            x(0, "code", Obj::to_type(STR_FURI)))
          ->inst_f([this](
          const Obj_p &, const InstArgs &args) {
              this->print_prompt();
              Terminal::STD_OUT_DIRECT(
                str(StringHelper::format(
                  "%s\n",
                  args->arg(0)->str_value().c_str())));
              string code = args->arg(0)->str_value();
              if(FOS_IS_DOC_BUILD)
                StringHelper::replace(&code, "\\|", "|");
              this->process_line(code);
              return noobj();
            })
          ->create()},
        {"config", config}}),
      THREAD_FURI,
      id_p(value_id))) {
    }

  public:
    static ptr<Console> create(const ID &id, const Rec_p &settings) {
      const auto console = ptr<Console>(new Console(id, settings));
      return console;
    }

    static void *import(const ID &id = "/io/lib/console") {
      // Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      InstBuilder::build(ID(id.extend(":create")))
          ->type_args(
            x(0, "install_location", vri(id)),
            x(1, "config", Obj::to_rec({
                {"terminal",
                  Obj::to_rec({
                    {"stdin", vri(Terminal::singleton()->vid()->extend(":stdin"))},
                    {"stdout", vri(Terminal::singleton()->vid()->extend(":stdout"))}})},
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
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
