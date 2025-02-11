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

#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../util/obj_helper.hpp"
#include "../structure/router.hpp"

#define OBJ_ID_WRAP "!g[!m%s!g]!!"
#define SYS_ID_WRAP "!g[!y%s!g]!!"

namespace fhatos {
  class Log final : public Rec {
  protected:
    explicit Log(const ID &value_id, const Rec_p &config) : Rec(rmap({{"config", config}}),
                                                                OType::REC,
                                                                REC_FURI,
                                                                id_p(value_id)) {
    }

    template<typename... Args>
    static void PRINT_LOG(const LOG_TYPE type, const Obj *source, const char *format, const Args... args) {
      std::lock_guard lock(stdout_mutex);
      if(type == NONE)
        printer<>()->print("");
      else if(type == ERROR)
        printer<>()->print("!r[ERROR]!! ");
      else if(type == WARN)
        printer<>()->print("!y[WARN] !! ");
      else if(type == INFO)
        printer<>()->print("!g[INFO] !! ");
      else if(type == DEBUG)
        printer<>()->print("!y[DEBUG]!! ");
      else if(type == TRACE)
        printer<>()->print("!r[TRACE]!! ");
      printer<>()->print((StringHelper::format((source->vid->equals(*Router::singleton()->vid)
                                                 /*|| source->vid->equals(*SCHEDULER_ID)*/)
                                                 ? SYS_ID_WRAP
                                                 : OBJ_ID_WRAP, // TODO: once scheduler.hpp and .cpp are split
                                               source->vid_or_tid()->toString().c_str()) + " ").c_str());
      printer<>()->print(StringHelper::format(format, args...).c_str());
    }

  public:
    template<typename... Args>
    static void LOGGER(const LOG_TYPE type, const Obj *source, const char *format, const Args... args) {
      bool match = false;
      const fURI furi = fURI("config").extend(LOG_TYPES.to_chars(type));
      for(const auto &a: *Log::singleton()->this_get(furi)->lst_value()) {
        if(source->vid_or_tid()->matches(a->uri_value())) {
          match = true;
          break;
        }
      }
      if(match) { // make it type based once fully integrated
        PRINT_LOG(INFO, source, format, args...);
      }
    }

    static ptr<Log> create(const ID &id, const Rec_p &config = noobj()) {
      const static auto log = ptr<Log>(new Log(id, config->is_noobj()
                                                     ? rec({
                                                       {"INFO", lst({vri("#")})},
                                                       {"ERROR", lst({vri("#")})},
                                                       {"DEBUG", lst()},
                                                       {"TRACE", lst()}})
                                                     : config));
      return log;
    }

    static ptr<Log> singleton(const ID &id = "/io/log") {
      const static ptr<Log> log = Log::create(id, Obj::to_noobj());
      return log;
    }

    static void *import(const ID &id = "/io/lib/log") {
      // ROUTER_WRITE(id_p(id), Obj::to_rec({{"allow", Obj::to_type(REC_FURI)}}), true);
      /*  InstBuilder::build(ID(id.extend("create")))
            ->domain_range(OBJ_FURI, {0, 1}, id_p(id), {1, 1})
            ->type_args(
              x(0, "install_location", vri(id)),
              x(1, "config", noobj()))
            ->save();*/
      return nullptr;
    }
  };
} // namespace fhatos

#endif
