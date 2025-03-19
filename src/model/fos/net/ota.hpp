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
#ifdef ESP_PLATFORM
#ifndef fhatos_ota_hpp
#define fhatos_ota_hpp

#include "../../../fhatos.hpp"
#include <ArduinoOTA.h>
#include  "../sys/scheduler/thread/thread.hpp"

#define DEFAULT_OTA_PORT 3232

namespace fhatos {
  const ID_p OTA_FURI = id_p(FOS_URI "/net/ota");

  class OTA : public Model<OTA> {
  public:
    explicit OTA() {
    }

    static ptr<OTA> create_state(const Obj_p &ota_obj) {
      ota_obj->rec_set("loop", Obj::to_inst(make_tuple<InstArgs, InstF, Obj_p>(
                                              Obj::to_inst_args(), InstF(make_shared<Cpp>(
                                                [](const Obj_p &, const InstArgs &) {
                                                  ArduinoOTA.handle();
                                                  return Obj::to_noobj();
                                                })),
                                              Obj::to_noobj()), INST_FURI));
      ArduinoOTA.setHostname(ota_obj->get<fURI>("config/host").host());
      ArduinoOTA.setPort(ota_obj->get<fURI>("config/host").port());
      ArduinoOTA.setMdnsEnabled(ota_obj->get<fURI>("config/host").scheme() == "mdns");
      ArduinoOTA.onStart([ota_obj]() {
        ota_obj->rec_get("config/on_start")->apply(Obj::to_noobj());
      });
      ArduinoOTA.onProgress([ota_obj](const unsigned int a, const unsigned int b) {
        ota_obj->rec_get("config/on_progress")->apply(Obj::to_noobj(), rec({{"a", jnt(a)}, {"b", jnt(b)}}));
      });
      ArduinoOTA.onError([ota_obj](const ota_error_t error) {
        ota_obj->rec_get("config/on_error")->apply(Obj::to_noobj());
      });
      ArduinoOTA.onEnd([ota_obj]() {
        ota_obj->rec_get("config/on_end")->apply(Obj::to_noobj());
      });
      ArduinoOTA.begin();
      return make_shared<OTA>();
    }

    static Rec_p obj(const std::initializer_list<Pair<const string, Obj_p>> &map, const ID &value_id) {
      return Obj::to_rec(map, OTA_FURI, id_p(value_id));
    }

    static Obj_p start_inst(const Obj_p &ota_obj, const InstArgs &) {
      Model<OTA>::get_state<OTA>(ota_obj);
      string ota_str = PrintHelper::pretty_print_obj(ota_obj, 3);
      StringHelper::prefix_each_line(FOS_TAB_1, &ota_str);
      LOG_OBJ(INFO, ota_obj, "\n%s\n", ota_str.c_str());
      return ota_obj;
    }

    static Obj_p stop_inst(const Obj_p &, const InstArgs &) {
      ArduinoOTA.end();
      return Obj::to_noobj();
      //Thread::stop();
    }

    static void *import(const Obj_p &ota_config = nullptr) {
      //////////////////////
      Typer::singleton()->save_type(*OTA_FURI, Obj::to_rec({
                                      {"halt", Obj::to_type(BOOL_FURI)},
                                      {"config", Obj::to_rec({
                                        {"host", Obj::to_type(URI_FURI)}})}
                                    }));
      ///////////////////////////////////////////////////////
      InstBuilder::build(OTA_FURI->add_component("start"))
          ->domain_range(OTA_FURI, {1, 1}, OTA_FURI, {1, 1})
          ->inst_f([](const Obj_p &ota_obj, const InstArgs &args) {
            return OTA::start_inst(ota_obj, args);
          })->save();
      InstBuilder::build(OTA_FURI->add_component("stop"))
          ->domain_range(OTA_FURI, {1, 1}, NOOBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &ota_obj, const InstArgs &args) {
            return OTA::stop_inst(ota_obj, args);
          })->save();
      ///////////////////////////////////////////////////////
      if(ota_config && !ota_config->is_noobj()) {
        const Obj_p &ota_obj = OTA::obj({{"halt", dool(false)}, {"config", ota_config->clone()}}, "/io/ota");
        LOG_OBJ(INFO, ota_obj, "!yota startup!! attempt: !b%s!!\n",
                ota_config->rec_get("host")->uri_value().toString().c_str());
        //OTA::start_inst(ota_obj, Obj::to_inst_args());
      }
      return nullptr;
    }
  };
} // namespace fhatos

#endif
#endif
