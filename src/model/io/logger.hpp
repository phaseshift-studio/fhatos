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
#ifndef fhatos_logger2_hpp
#define fhatos_logger2_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <furi.hpp>
#include <util/string_helper.hpp>
#include <util/obj_helper.hpp>

namespace fhatos {
  static const auto LOG_TYPES2 = Enums<LOG_TYPE>({{ALL, "ALL"},
    {TRACE, "TRACE"},
    {DEBUG, "DEBUG"},
    {INFO, "INFO"},
    {WARN, "WARN"},
    {ERROR, "ERROR"},
    {NONE, "NONE"}});

  class Logger2 final : public Rec {
  public:
    using Settings = Inst_p;


    enum LOG_TYPE { ALL = 0, TRACE = 1, DEBUG = 2, INFO = 3, WARN = 4, ERROR = 5, NONE = 6 };

  protected:
    Inst_p settings_;

    explicit Logger2(const ID &value_id) : Rec(rmap({{}}), OType::REC, id_p("/io/lib/logger"), id_p(value_id)),
                                           settings_(from(vri(value_id.extend("config")))) {
      this->rec_set("config", Obj::to_rec({{"track", Obj::to_rec({{vri("#"), vri("INFO")}})}}));
      this->at(id_p(value_id));
    }

  public:
    static ptr<Logger2> create(const ID &id) {
      const auto logger = ptr<Logger2>(new Logger2(id));
      return logger;
    }

    static void MAIN_LOG(const LOG_TYPE type, const char *format, ...) {
      if(static_cast<uint8_t>(type) < static_cast<uint8_t>(Options::singleton()->log_level<LOG_TYPE>()))
        return;
      va_list arg;
      va_start(arg, format);
      char *buffer;
      const size_t length = vasprintf(&buffer, format, arg);
      va_end(arg);
      if(format[strlen(format) - 1] == '\n')
        buffer[length - 1] = '\n';
      buffer[length] = '\0';
      // control garbled concurrent writes (destructor releases lock)
      std::lock_guard<std::mutex> lock(stdout_mutex);
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
      printer<>()->print(buffer);
      free(buffer);
    }

    static void *import(const ID &id = "/io/lib/logger",
                        const Rec_p &config = Obj::to_rec({
                          {"track", Obj::to_rec({{vri("#"), vri("INFO")}})}
                        })) {
      InstBuilder::build(ID(id.extend(C_INST_C).extend(id.extend("create"))))
          ->type_args(
            x(0, "install_location", vri("/io/logger")),
            x(1, "config", config))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            ptr<Logger2> logger = Logger2::create(args->arg(0)->uri_value());
            args->arg(1)->at(id_p(logger->vid_->extend("config")));
            return logger;
          })
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
