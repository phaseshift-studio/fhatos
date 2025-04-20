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
#ifndef fhatos_terminal_hpp
#define fhatos_terminal_hpp

#include "../../../fhatos.hpp"

namespace fhatos {
  static const ID_p TERMINAL_FURI = id_p(FOS_URI "/ui/terminal");

  class Terminal final : public Rec {
  protected:
    explicit Terminal(const ID &id) :
      Rec({}, OType::REC, TERMINAL_FURI, id_p(id)) {
    }

  public:
    static void *import() {
      Typer::singleton()->save_type(*TERMINAL_FURI, Obj::to_rec());
      InstBuilder::build(TERMINAL_FURI->add_component(":stdout"))
          ->domain_range(TERMINAL_FURI, {0, 1}, OBJ_FURI, {0, 0})
          ->inst_args(rec({{"output?str", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            FEED_WATCHDOG();
            const Str_p output = args->arg("output")->clone();
            STD_OUT_DIRECT(output);
            return noobj();
          })->save();
      InstBuilder::build(TERMINAL_FURI->add_component(":stdin"))
          ->domain_range(TERMINAL_FURI, {0, 1}, STR_FURI, {0, 1})
          ->inst_args(rec({{"until?str", __().else_(str("\n"))}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return STD_IN_LINE_DIRECT(args->arg("until")->str_value()[0]);
          })
          ->save();
      return nullptr;
    }

    static ptr<Terminal>& singleton(const ID &id = ID("/io/terminal")) {
      static auto terminal_p = ptr<Terminal>(new Terminal(id));
      return terminal_p;
    }

    static void STD_OUT_DIRECT(const Str_p &str, const Int_p &ellipsis = Obj::to_noobj()) {
      static auto mutex_ = Mutex();
      auto lock = std::lock_guard(mutex_);
      auto output = string(str->str_value());
      //if(!ellipsis->is_noobj())
      //  StringHelper::truncate(output, ellipsis->int_value() + (output.length() - Ansi<>::strip(output).length()));
      printer<>()->print(output.c_str());
    }

    static Int_p STD_IN_DIRECT() {
      int c;
      while(-1 == (c = printer<>()->read())) {
        Thread::yield_current_thread();
      }
      return jnt(c);
    }

    static Str_p STD_IN_LINE_DIRECT(const char until) {
      string line;
      int c = EOF;
      while(c != until) {
        while(/*!ROUTER_READ(SCHEDULER_ID->extend("halt"))->or_else(dool(false))->bool_value() &&*/ // TODO: create a global shutdown flag at /sys/halt
              -1 == (c = printer<>()->read())) {
          Thread::yield_current_thread();
        }
        line += static_cast<char>(c);
        if(until == '\0')
          break;
      }
      return str(line);
    }
  };
} // namespace fhatos
#endif
