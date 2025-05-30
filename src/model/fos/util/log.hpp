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
#include "../../../lang/mmadt/mmadt.hpp"
#include "../../../lang/obj.hpp"
#include "../../../model/fos/sys/scheduler/thread/mutex.hpp"
#include "../../../util/obj_helper.hpp"
#include "../../model.hpp"

#define OBJ_ID_WRAP "!g[!m{}!g]!! "
#define SYS_ID_WRAP "!g[!y{}!g]!! "
#define NONE_ID_WRAP "!g[!r{}!g]!! "

namespace fhatos {
  using namespace mmadt;
  static const ID_p LOG_FURI = id_p("/fos/util/log");
  static ptr<Trip<const LOG_TYPE, const Obj *, const std::function<std::string()>>> log_stack;

  struct Entry {
    const LOG_TYPE type;
    const Obj *source;
    const std::function<std::string()> message;

    Entry() = delete;
    Entry(const Entry &other) = delete;
    Entry &operator=(const Entry &other) = delete;

    Entry(const LOG_TYPE log_type, const Obj *log_source, std::function<std::string()> log_message) :
        type(log_type), source(log_source), message(std::move(log_message)) {}

    Entry(const LOG_TYPE log_type, std::function<std::string()> log_message) :
        Entry(log_type, nullptr, std::move(log_message)) {}

    void log() const { LOG_WRITE(this->type, this->source, this->message); }
  };

  class Log final : public Rec {
  protected:
    Mutex log_mutex;

  public:
    explicit Log(const Obj_p &log_obj) : Rec(*log_obj) {
      printer<>()->printf("!g[INFO]  [!m%s!g] !yboot logger!g/!ysystem logger!! swapped\n",
                          this->Obj::vid_or_tid()->toString().c_str());
      LOG_WRITE = [](const LOG_TYPE log_type, const Obj *source, const std::function<std::string()> &message) {
        PRIMARY_LOGGING(log_type, source, message);
        /*MemoryHelper::use_custom_stack(InstBuilder::build("log_helper")->inst_f(
                                           [log_type, &message](const Obj_p &obj, const InstArgs &) {

                                             return Obj::to_noobj();
                                           })->create(), source->shared_from_this(), 8000, true);*/
      };
    }

    static void PRIMARY_LOGGING(const LOG_TYPE type, const Obj *source, const std::function<std::string()> &message) {
      const auto logging = fURI(string("config/").append(LOG_TYPES.to_chars(type)));
      const Lst_p furis = Log::singleton()->obj_get(logging)->or_else(lst());
      if(!furis->is_lst()) {
        printer()->print(fmt::format("!r[ERROR] !! " OBJ_ID_WRAP
                                     " log listing not within schema specification: !b{}!!\n",
                                     LOG_FURI ? LOG_FURI->toString() : "<none>", Log::singleton()->toString())
                             .c_str());
        return;
      }
      bool match = false;
      const auto furis_list = std::vector<Obj_p>(*furis->lst_value());
      if(nullptr != source) {
        for(const auto &a: furis_list) {
          if(source->vid_or_tid()->matches(a->uri_value())) {
            match = true;
            break;
          }
        }
      }
      if(!match)
        return;
      // auto lock = lock_guard<Mutex>(Log::singleton()->log_mutex);
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
      if(source->vid && source->vid->has_path("sys"))
        printer()->print(fmt::format(SYS_ID_WRAP, source->vid_or_tid()->toString()).c_str());
      else if(source->vid)
        printer()->print(fmt::format(OBJ_ID_WRAP, source->vid_or_tid()->toString()).c_str());
      else
        printer()->print(fmt::format(NONE_ID_WRAP, "<none>").c_str());
      printer()->print(message().c_str());
    }

    static ptr<Log> singleton(const ID &id = "/io/log") {
      const static auto log = make_shared<Log>(Obj::to_rec({{"config", rec({{"INFO", lst({vri("#")})},
                                                                            {"ERROR", lst({vri("#")})},
                                                                            {"DEBUG", lst()},
                                                                            {"WARN", lst()},
                                                                            {"TRACE", lst()}})}},
                                                           LOG_FURI, id_p(id)));
      return log;
    }

    /* static Obj_p log_inst(const Obj_p &source_obj, const InstArgs &args) {
       string log_level_str = args->get<fURI>("level").toString();
       std::transform(log_level_str.begin(), log_level_str.end(), log_level_str.begin(), ::toupper);
       const LOG_TYPE log_level = LOG_TYPES.to_enum(log_level_str);
       Log::PRIMARY_LOGGING(log_level, source_obj.get(), L("{}", args->arg("message")->toString()));
       return source_obj;
     }*/

  public:
    static void *import() {
      MODEL_CREATOR2->insert_or_assign(*LOG_FURI, [](const Obj_p &log_obj) { return make_shared<Log>(log_obj); });
      ////////////////////////// TYPE ////////////////////////////////
      TYPER_SAVE_TYPE(LOG_FURI->extend("level"),
                 __(LOG_FURI->extend("level"), *URI_FURI, *URI_FURI)
                     .is(__().or_(__().eq(vri("INFO")), __().eq(vri("ERROR")), __().eq(vri("DEBUG")),
                                  __().eq(vri("WARN")), __().eq(vri("TRACE")))));
      TYPER_SAVE_TYPE(*LOG_FURI, Obj::to_rec({{"config", __().else_(rec({{"INFO", __().else_(lst({vri("#")}))},
                                                                    {"ERROR", __().else_(lst({vri("#")}))},
                                                                    {"DEBUG", __().else_(lst())},
                                                                    {"WARN", __().else_(lst())},
                                                                    {"TRACE", __().else_(lst())}}))}}));
      ////////////////////////// INSTS ////////////////////////////////
      InstBuilder::build(LOG_FURI->add_component("log"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(
              rec({{vri(ID("level").query(LOG_FURI->extend("level").toString().c_str())), __().else_(vri("INFO"))},
                   {vri("message?str"), Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &source, const InstArgs &args) {
            const LOG_TYPE log_level = LOG_TYPES.to_enum(args->get<fURI>("level").toString());
            const auto message = args->arg("message")->str_value();
            Log::PRIMARY_LOGGING(log_level, source.get(), L("{}\n", message));
            return source;
          })
          ->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
