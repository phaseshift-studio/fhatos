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
#ifndef fhatos_log_hpp
#define fhatos_log_hpp

#include <fhatos.hpp>
#include <furi.hpp>

namespace fhatos {
  class Log final : public Rec {
  protected:
    ///// printers
    void write_stdout(const Str_p &s) const {
      //if(this->direct_stdin_out)
      //Terminal::STD_OUT_DIRECT(s);
      //else
      // ROUTER_WRITE(this->this_get("config/terminal/stdout")->uri_p_value<ID>(), s, TRANSIENT);
    }


    explicit Log(const ID &value_id, const Rec_p &settings) : Rec(settings->rec_value(),
                                                                  OType::REC,
                                                                  REC_FURI,
                                                                  id_p(value_id)) {
      this->save();
    }

  public:
    template<typename... Args>
    static void LOGGER(const LOG_TYPE type, const Obj_p source, const char *format, const Args... args) {
      bool match = false;
      const fURI furi = fURI("allow").extend(LOG_TYPES.to_chars(type));
      for(const auto &a: *Log::singleton()->
          this_get(furi)->lst_value()) {
        if(source->vid_or_tid()->matches(a->uri_value())) {
          match = true;
          break;
        }
      }
      if(match) { // make it type once fully integrated
        LOG_OBJ(INFO, source, format, args...);
      }
    }


    static ptr<Log> create(const ID &id, const Rec_p &config = noobj()) {
      static Rec_p DEFAULT_CONFIG = Obj::to_rec({
        /*  {"terminal",
            Obj::to_rec({
              {"stdin", vri(Terminal::singleton()->vid()->extend(":stdin"))},
              {"stdout", vri(Terminal::singleton()->vid()->extend(":stdout"))}})},*/
        {"allow", Obj::to_rec({
          {"INFO", lst({vri("#")})},
          {"ERROR", lst({vri("#")})},
          {"DEBUG", lst()},
          {"TRACE", lst()}})}
      });
      const auto log = ptr<Log>(new Log(id, config->is_noobj() ? DEFAULT_CONFIG : config));
      return log;
    }

    static ptr<Log> singleton(const ID &id = "/io/log") {
      const static ptr<Log> log = Log::create(id, Obj::to_noobj());
      return log;
    }

    static void *import(const ID &id = "/io/lib/log") {
      // Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      InstBuilder::build(ID(id.extend(":create")))
          ->type_args(
            x(0, "install_location", vri(id)),
            x(1, "config", noobj()))
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
