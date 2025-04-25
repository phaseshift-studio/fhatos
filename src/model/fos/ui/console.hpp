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

#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../../lang/mmadt/parser.hpp"
#include "../../model.hpp"
#include "terminal.hpp"
#include "../../../lang/type.hpp"
#include "../sys/scheduler/thread/thread.hpp"
#include "../../../../extern/fmt/include/fmt/core.h"

namespace fhatos {
  const ID_p CONSOLE_FURI = id_p(FOS_URI "/ui/console");

  class Console final : public Thread, public Rec {
  protected:
    string line_;
    bool new_input_ = true;
    mmadt::Tracker tracker_;
    bool first = true;

  public:
    explicit Console(const Obj_p &console_obj) :
      Thread(console_obj), Rec(*console_obj) {
    }

    static Obj_p create(const ID &id, const Rec_p &console_config) {
      const Obj_p console_obj =
          Obj::to_rec({
                          {"halt", dool(false)},
                          {"delay", jnt(0, NAT_FURI)},
                          {"loop", InstBuilder::build(THREAD_FURI->extend("loop"))
                           ->domain_range(CONSOLE_FURI, {1, 1}, OBJ_FURI, {0, 1})
                           ->inst_f([](const Obj_p &console_obj, const InstArgs &) {
                             // static_cast necessary for esp32
                             const auto console_state = static_cast<Console *>(get_state<Thread>(console_obj).get());
                             if(console_state->first) {
                               console_state->first = false;
                               Thread::delay_current_thread(300);
                               printer()->println();
                             }
                             try {
                               /// WRITE TO PROMPT
                               if(console_obj->has("config/terminal/stdout")) {
                                 if(console_state->new_input_)
                                   console_state->print_prompt(!console_state->line_.empty());
                                 console_state->new_input_ = false;
                               }
                               if(console_obj->has("config/terminal/stdin")) {
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
                                 StringHelper::trim(console_state->line_);
                                 console_state->process_line(console_state->line_);
                                 console_state->line_.clear();
                               }
                             } catch(const fError &e) {
                               console_state->print_exception(e);
#ifdef NATIVE
                                if(console_obj->rec_get("config/stack_trace", dool(false))->bool_value()) {
                                  console_state->write_stdout(str("\t!yprint stack trace!! !m[!gy!m/!gN!m]!y?!! "));
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
                           })->create()},
                          {"config", console_config->clone()}}, CONSOLE_FURI, id_p(id));
      MODEL_STATES::singleton()->store(id, Console::create_state(console_obj));
      //Scheduler::singleton()->spawn_thread(console_obj);
      // __().inst(Scheduler::singleton()->vid->add_component("spawn"), __().block(console_obj)).compute();
      return console_obj;
    }

    static ptr<Thread> create_state(const Obj_p &console_obj) {
      ptr<Thread> console_state = make_shared<Console>(console_obj);
      return console_state;
    }

    void clear() {
      this->tracker_.clear();
      this->line_.clear();
      this->new_input_ = true;
    }

    ///// printers
    void write_stdout(const Str_p &s) const {
      if(this->has("config/terminal/stdout")) {
        if(const fURI out = this->get<fURI>("config/terminal/stdout");
          Terminal::singleton()->vid->extend(":stdout") == out) {
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
      return Terminal::singleton()->vid->extend(":stdin") == in
               ? Terminal::STD_IN_LINE_DIRECT(until)
               : Router::singleton()->exec(in, noobj());
      //}
    }


    void print_exception(const std::exception &ex) const {
      this->write_stdout(str(fmt::format("!r[ERROR]!! {}\n", ex.what())));
    }

    void print_prompt(const bool blank = false) const {
      const auto prompt = this->get<string>("config/prompt");
      this->write_stdout(str(blank ? StringHelper::repeat(Ansi<>::strip(prompt).length()) : prompt));
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
      LOG_WRITE(DEBUG, this, L("line to parse: {}\n", line));
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
      const static Uri_p processor_uri = Obj::to_uri(*PROCESSOR_FURI);
      if(const Uri_p proc = this->rec_get("config/processor"); !proc->equals(*processor_uri))
        result = __().inst(proc->uri_value().add_component("eval"), str(line)).compute().to_objs();
      else
        result = Processor::compute(line);
      std::stringbuf to_out;
      FEED_WATCHDOG();
      PrintHelper::pretty_print_obj(result, 0, this->get<int>("config/nest"), false, &to_out);
      this->write_stdout(str(to_out.str()));
    }

    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      Typer::singleton()->save_type(
          *CONSOLE_FURI, Obj::to_rec({
              {"delay", __().isa(*NAT_FURI)},
              {"loop", __()},
              {"halt", __().isa(*BOOL_FURI)}
          }));
      InstBuilder::build(CONSOLE_FURI->add_component("clear"))
          ->domain_range(CONSOLE_FURI, {1, 1}, CONSOLE_FURI, {1, 1})
          ->inst_f([](const Obj_p &console_obj, const InstArgs &) {
            const ptr<Thread> console_state = Model::get_state<Thread>(console_obj);
            static_cast<Console *>(console_state.get())->clear();
            return console_obj;
          })->save();
      InstBuilder::build(CONSOLE_FURI->add_component("eval"))
          ->domain_range(CONSOLE_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_args(rec({{"code", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &console_obj, const InstArgs &args) {
            string code = args->arg("code")->str_value();
            StringHelper::replace(&code, "\\'", "\'"); // unescape quotes (should this be part of str?)
            const ptr<Thread> console_state = Model::get_state<Thread>(console_obj);
            static_cast<Console *>(console_state.get())->process_line(code);
            return Obj::to_noobj();
          })
          ->save();
      return nullptr;
    }
  };

} // namespace fhatos
#endif
