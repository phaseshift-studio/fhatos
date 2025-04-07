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

#include <memory>

#include "../../../fhatos.hpp"
#include "../../../furi.hpp"
#include "../../../lang/obj.hpp"
#include "../../../lang/mmadt/mmadt.hpp"
#include "../../../util/obj_helper.hpp"

#define OBJ_ID_WRAP "!g[!m{}!g]!! "
#define SYS_ID_WRAP "!g[!y{}!g]!! "

namespace fhatos {
  using namespace mmadt;
  static const ID_p LOG_FURI = id_p("/fos/util/log");

  class Log final : public Rec {
  public:
    explicit Log(const ID &value_id, const Rec_p &config) :
      Rec(rmap({{"config", config}}), OType::REC, LOG_FURI, id_p(value_id)) {
      printer<>()->printf("!g[INFO]  [!m%s!g] switching from !yboot logger!! to !ysystem logger!!\n",
                          this->Obj::vid_or_tid()->toString().c_str());
      LOG_WRITE = [](const LOG_TYPE log_type, const Obj *source, const std::function<std::string()> &message) {
        PRIMARY_LOGGING(log_type, source, message);
      };
    }

    static void PRIMARY_LOGGING(const LOG_TYPE type, const Obj *source, const std::function<std::string()> &message) {
      const Lst_p furis = Log::singleton()->rec_get("config/" + LOG_TYPES.to_chars(type));
      if(!furis->is_lst()) {
        printer<>()->print(fmt::format(
            "!r[ERROR] !! " OBJ_ID_WRAP " log listing not within schema specification: !b{}!!\n",
            LOG_FURI->toString(),
            Log::singleton()->toString()).c_str());
        return;
      }
      bool match = false;
      const bool source_is_null = nullptr == source;
      for(const auto &a: *furis->lst_value()) {
        if(!source_is_null && source->vid_or_tid()->matches(a->uri_value())) {
          match = true;
          break;
        }
      }
      if(!match)
        return;
      std::lock_guard lock(stdout_mutex);
      if(type == NONE)
        printer()->print("");
      else if(type == INFO)
        printer()->print("!g[INFO] !! ");
      else if(type == ERROR)
        printer()->print("!r[ERROR]!! ");
      else if(type == WARN)
        printer()->print("!y[WARN] !! ");
      else if(type == DEBUG)
        printer()->print("!y[DEBUG]!! ");
      else if(type == TRACE)
        printer()->print("!r[TRACE]!! ");
      if(source_is_null || (source->vid && source->vid->has_path("sys")))
        printer()->print(
            fmt::format(SYS_ID_WRAP, source_is_null ? "<none>" : source->vid_or_tid()->toString()).c_str());
      else
        printer()->print(fmt::format(OBJ_ID_WRAP, source->vid_or_tid()->toString()).c_str());
      printer()->print(message().c_str());
    }

    static ptr<Log> create(const ID &id, const Rec_p &config = noobj()) {
      const static auto log = std::make_shared<Log>(id, config->is_noobj()
                                                          ? rec({
                                                              {"INFO", lst({vri("#")})},
                                                              {"ERROR", lst({vri("#")})},
                                                              {"DEBUG", lst()},
                                                              {"WARN", lst()},
                                                              {"TRACE", lst()}})
                                                          : config);
      return log;
    }

    static ptr<Log> singleton(const ID &id = "/io/log") {
      const static ptr<Log> log = Log::create(id, Obj::to_noobj());
      return log;
    }

    static Obj_p log_inst(const Obj_p &source_obj, const InstArgs &args) {
      string log_level_str = args->get<fURI>("level").toString();
      std::transform(log_level_str.begin(), log_level_str.end(), log_level_str.begin(), ::toupper);
      const LOG_TYPE log_level = LOG_TYPES.to_enum(log_level_str);
      Log::PRIMARY_LOGGING(log_level, source_obj.get(),L("{}", args->arg("message")->toString()));
      return source_obj;
    }

  public:
    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      TYPE_SAVER(*LOG_FURI, Obj::to_rec({{"config", rec({
                                              {"INFO", Obj::to_type(LST_FURI)},
                                              {"ERROR", Obj::to_type(LST_FURI)},
                                              {"DEBUG", Obj::to_type(LST_FURI)},
                                              {"WARN", Obj::to_type(LST_FURI)},
                                              {"TRACE", Obj::to_type(LST_FURI)}})}}));
      ////////////////////////// INSTS ////////////////////////////////
      InstBuilder::build(LOG_FURI->add_component("log"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(rec({{"level", __().is(__().or_(__().eq(vri("INFO")),
                                                      __().eq(vri("ERROR")),
                                                      __().eq(vri("DEBUG")),
                                                      __().eq(vri("WARN")),
                                                      __().eq(vri("TRACE"))))},
                           {"message?str", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &source, const InstArgs &args) {
            const LOG_TYPE log_level = LOG_TYPES.to_enum(args->get<fURI>("level").toString());
            auto message = args->arg("message")->str_value();
            Log::PRIMARY_LOGGING(log_level, source.get(),L("{}\n", message));
            return source;
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
