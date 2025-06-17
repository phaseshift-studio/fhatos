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
#ifndef fhatos_ui_console_hpp
#define fhatos_ui_console_hpp

#include "../../../../extern/fmt/include/fmt/core.h"
#include "../../../fhatos.hpp"
#include "../../../lang/mmadt/parser.hpp"
#include "../../../lang/obj.hpp"
#include "../../../structure/pubsub.hpp"
#include "../../../util/print_helper.hpp"
#include "../../module.hpp"
#include "../sys/scheduler/scheduler.hpp"
#include "../sys/scheduler/thread/thread.hpp"
#include "../sys/typer/typer.hpp"
#include "terminal.hpp"
#ifdef NATIVE
#include <cpptrace/cpptrace.hpp>
#endif

#define CONSOLE_TID FOS_URI "/ui/console"

namespace fhatos {
  const ID_p CONSOLE_FURI = id_p(FOS_URI "/ui/console");

  class Console final : public Rec, Mailbox {
  protected:
    string line_;
    bool new_input_ = true;
    mmadt::Tracker tracker_;
    bool first = true;

  public:
    explicit Console(const Obj_p &console_obj) : Rec(*console_obj) {}

    /*static Obj_p create(const ID &id, const Rec_p &console_config) {
      return Obj::to_rec({{"config", console_config}}, CONSOLE_FURI, id_p(id));
    }*/

    static ptr<Console> create_state(const Obj_p &console_obj) {
      const auto console_state = make_shared<Console>(console_obj);
      return console_state;
    }

    void clear() {
      this->tracker_.clear();
      this->line_.clear();
      this->new_input_ = true;
    }

    virtual bool recv_payload(const void *payload) override {
      this->recv_mail(std::move(Mail(*(Mail *) payload)));
      return true;
    }

    ///// printers
    void write_stdout(const Str_p &s) const {
      if(this->has("config/terminal/stdout")) {
        if(const fURI out = this->get<fURI>("config/terminal/stdout");
           Terminal::singleton()->vid->add_component("stdout") == out) {
          Terminal::STD_OUT_DIRECT(s, this->rec_get("config/ellipsis"));
        } else
          ROUTER_WRITE(out, s, TRANSIENT);
      } else {
        Terminal::STD_OUT_DIRECT(s, this->rec_get("config/ellipsis"));
      }
    }

    [[nodiscard]] Str_p read_stdin(const char until) const {
      // if(console_obj->has("config/terminal/stdin")) {
      const fURI in = this->get<fURI>("config/terminal/stdin");
      return Terminal::singleton()->vid->add_component("stdin") == in ? Terminal::STD_IN_LINE_DIRECT(until)
                                                                      : ROUTER_READ(in);
      //}
    }

    void print_exception(const std::exception &ex) const {
      this->write_stdout(str(fmt::format("!r[ERROR]!! {}\n", ex.what())));
    }

    void print_prompt(const bool blank = false) const {
      const auto prompt = this->get<string>("config/prompt");
      this->write_stdout(str(blank ? StringHelper::repeat(Ansi<>::strip(prompt).length()) : prompt));
    }

    void process_output(const Obj_p &result) const {
      std::stringbuf to_out;
      PrintHelper::pretty_print_obj(result, 0, this->obj_get("config/nest")->or_else_<int>(2), false, &to_out);
      this->write_stdout(str(to_out.str()));
    }

    void process_input(string line) const {
      /////////////////////////////////////////////////////////////
      ////////////// EXPERIMENTING WITH ANSI MOVEMENT /////////////
      if(line == "TEST") {
        this->write_stdout(str("\n\n\n!^u2"));
        this->write_stdout(str("!^S1!y1 3 5 7 9!^L1!^d1!g2 4 6 8!!\n"));
        return;
      }
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      // LOG_WRITE(DEBUG, this, L("line to parse: {}\n", line));
      StringHelper::trim(line);
      ///////// PARSE OBJ AND IF BYTECODE, EXECUTE IT
      if(line[0] == '\n')
        line = line.substr(1);
      if(line.empty()) {
        this->write_stdout(str("\n"));
        return;
      }
      FEED_WATCHDOG();

      Obj_p result;
      if(const Uri_p proc = this->rec_get("config/processor")->or_else(vri(PROCESSOR_TID));
         !proc->uri_value().equals(PROCESSOR_TID))
        result = __().inst(proc->uri_value().add_component("eval"), str(line)).compute().to_objs();
      else
        result = Processor::compute(line);
      this->process_output(result);
    }

    static void register_module() {
      Typer::singleton()->register_module(
          CONSOLE_TID,
          InstBuilder::build(Typer::singleton()->vid->add_component(CONSOLE_TID))
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                // TODO: move to its own module?
                Terminal::import();
                MODEL_CREATOR2->insert_or_assign(
                    CONSOLE_TID, [](const Obj_p &console_obj) { return make_shared<Console>(console_obj); });
                return Obj::to_rec(
                    {{vri(CONSOLE_FURI),
                      Obj::to_rec(
                          {{vri("loop"),
                            __().lift(
                                InstBuilder::build(THREAD_FURI->extend("loop"))
                                    ->domain_range(id_p(CONSOLE_TID), {1, 1}, OBJ_FURI, {0, 1})
                                    ->inst_f([](const Obj_p &console_obj, const InstArgs &) {
                                      Console *console_state = console_obj->get_model<Console>();
                                      if(console_state->first) {
                                        console_state->first = false;
                                        Thread::delay(300);
                                        printer()->println();
                                      }
                                      try {
                                        console_state->Mailbox::loop();
                                        /// WRITE TO PROMPT
                                        if(!console_obj->rec_get("config/terminal/stdout")->is_noobj()) {
                                          if(console_state->new_input_)
                                            console_state->print_prompt(!console_state->line_.empty());
                                          console_state->new_input_ = false;
                                        }
                                        if(!console_obj->rec_get("config/terminal/stdin")->is_noobj()) {
                                          //// READ FROM PROMPT
                                          if(const string x = console_state->read_stdin('\n')->str_value();
                                             x.find(":clear") == 0) {
                                            console_state->clear();
                                            return noobj();
                                          } else {
                                            console_state->tracker_.track(x);
                                            if(console_state->tracker_.closed()) {
                                              console_state->new_input_ = true;
                                              console_state->line_ += x;
                                            } else {
                                              console_state->line_ += x;
                                              return Obj::to_noobj();
                                            }
                                            StringHelper::trim(console_state->line_);
                                            if(console_state->line_.empty() ||
                                               console_state->line_[console_state->line_.length() - 1] == ';' ||
                                               // specific to end-step and imperative simulation
                                               !console_state->tracker_.closed()) {
                                              ///////// DO NOTHING ON OPEN EXPRESSION (i.e. multi-line expressions)
                                              return noobj();
                                            }
                                          }
                                          // prepare the user input for processing
                                          console_state->tracker_.clear();
                                          console_state->process_input(console_state->line_);
                                          console_state->line_.clear();
                                        }
                                      } catch(const fError &e) {
                                        console_state->print_exception(e);
#ifdef NATIVE
                                        if(console_obj->rec_get("config/stack_trace", dool(false))->bool_value()) {
                                          console_state->write_stdout(
                                              str("\t!yprint stack trace!! !m[!gy!m/!gN!m]!y?!! "));
                                          string response = console_state->read_stdin('\0')->str_value();
                                          StringHelper::lower_case(response);
                                          if(response[0] == 'y') {
                                            const cpptrace::stacktrace st = cpptrace::generate_trace();
                                            console_state->write_stdout(str(st.to_string(true).append("\n")));
                                          }
                                        }
#endif
                                        console_state->clear();
                                      }
                                      return Obj::to_noobj();
                                    })
                                    ->create())},
                           {vri("halt"), __().else_(dool(false))},
                           {vri("config"),
                            __().else_(Obj::to_rec(
                                {{"nest", jnt(2)},
                                 {"prompt", str("!mfhatos!g>!! ")},
                                 {"stack_trace", dool(true)},
                                 {"strict", dool(false)},
                                 {"log", vri(LOG_TYPES.to_chars(INFO))},
                                 {"ellipsis", jnt(50)},
                                 {"terminal", Obj::to_rec({{"stdout", Obj::to_uri("/io/terminal/::/stdout")},
                                                           {"stdin", Obj::to_uri("/io/terminal/::/stdin")}})}}))}})},
                     {vri(CONSOLE_FURI->add_component("clear")),
                      InstBuilder::build(CONSOLE_FURI->add_component("clear"))
                          ->domain_range(CONSOLE_FURI, {1, 1}, CONSOLE_FURI, {1, 1})
                          ->inst_f([](const Obj_p &console_obj, const InstArgs &) {
                            Console *console_state = console_obj->get_model<Console>();
                            console_state->clear();
                            return console_obj;
                          })
                          ->create()},
                     {vri(CONSOLE_FURI->add_component("eval")),
                      InstBuilder::build(CONSOLE_FURI->add_component("eval"))
                          ->domain_range(CONSOLE_FURI, {1, 1}, OBJ_FURI, {0, 1})
                          ->inst_args(rec({{"code", Obj::to_bcode()}}))
                          ->inst_f([](const Obj_p &console_obj, const InstArgs &args) {
                            console_obj->get_model<Console>()->process_input(args->arg("code")->str_value());
                            return Obj::to_noobj();
                          })
                          ->create()},
                     {vri(CONSOLE_FURI->add_component("output")),
                      InstBuilder::build(CONSOLE_FURI->add_component("output"))
                          ->domain_range(CONSOLE_FURI, {1, 1}, OBJ_FURI, {0, 1})
                          ->inst_args(rec({{"obj", Obj::to_bcode()}}))
                          ->inst_f([](const Obj_p &console_obj, const InstArgs &args) {
                            console_obj->get_model<Console>()->process_output(args->arg("obj"));
                            return Obj::to_noobj();
                          })
                          ->create()}});
              })
              ->create());
    }
  };

} // namespace fhatos
#endif
