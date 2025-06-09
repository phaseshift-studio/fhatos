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
#ifndef fhatos_ollama_server_hpp
#define fhatos_ollama_server_hpp
#ifdef NATIVE
#include "../../../../fhatos.hpp"
#include "../../../../lang/util/ollama.hpp"
#include "../../../../util/helper.hpp"

#define LLM_MODEL_TID FOS_URI "/llm/model"
#define OLLAMA_TID FOS_URI "/llm/ollama"
#define DEFAULT_OLLAMA_SERVER_REST "http://localhost:11434"

namespace fhatos {

  class OllamaServer final : Rec {

  public:
    explicit OllamaServer(const Rec_p &ollama_server_obj) : Rec(*ollama_server_obj) {}

    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          OLLAMA_TID,
          InstBuilder::build(Typer::singleton()->vid->add_component(OLLAMA_TID))
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                MODEL_CREATOR2->insert_or_assign(OLLAMA_TID, [](const Obj_p &server_obj) {
                  auto ollama_server = make_shared<OllamaServer>(server_obj);
                  ollama_server->setup();
                  return ollama_server;
                });
                //////////////////////////////////////////////////////////////////////////////////////////////////////
                return Obj::to_rec(
                    {{vri(OLLAMA_TID),
                      Obj::to_rec({{"model", __().else_(lst())},
                                   {"config", __().else_(Obj::to_rec({{"rest", vri("http://localhost:11434")}}))}})},
                     {vri(ID(OLLAMA_TID).add_component("setup")),
                      InstBuilder::build(ID(OLLAMA_TID).add_component("setup"))
                          ->inst_f([](const Obj_p &ollama_obj, const InstArgs &) {
                            const auto o = ollama_obj->get_model<OllamaServer>();
                            o->setup();
                            return o->load();
                          })
                          ->create()},
                     {vri(LLM_MODEL_TID), Obj::to_rec({{"name?uri", __()}, {"server?uri", __()}})},
                     {vri(ID(LLM_MODEL_TID).add_component("setup")),
                      InstBuilder::build(ID(LLM_MODEL_TID).add_component("setup"))
                          ->inst_f([](const Uri_p &model_obj, const InstArgs &args) {
                            if(model_obj->vid) {
                              Subscription::create(id_p(model_obj->vid), p_p(model_obj->vid->extend("#")),
                                                   [&model_obj](const Obj_p &payload, const InstArgs &) {
                                                     const Str_p response =
                                                         Obj::load(model_obj->vid)->inst_apply("query", {payload});
                                                     Terminal::singleton()->STD_OUT_DIRECT(response);
                                                     return Obj::to_noobj();
                                                   })
                                  ->post();
                            }
                            return model_obj;
                          })
                          ->create()},
                     {vri(ID(LLM_MODEL_TID).add_component("query")),
                      InstBuilder::build(ID(LLM_MODEL_TID).add_component("query"))
                          ->inst_args("message?str", __())
                          ->inst_f([](const Uri_p &model_obj, const InstArgs &args) {
                            const string server_endpoint = ROUTER_READ(model_obj->rec_get("server")->uri_value())
                                                               ->rec_get("config/rest")
                                                               ->uri_value()
                                                               .toString();
                            try {
                              auto request = ollama::request(model_obj->rec_get("name")->uri_value().toString(),
                                                             args->arg("message")->str_value(), ollama::json::parse(R"(
  {
    "tools": ["mmadt"]
  }
)"),
                                                             true);
                              const bool result =
                                  Ollama(server_endpoint)
                                      .generate(request, [](const ollama::response &response) -> bool {
                                        Terminal::singleton()->STD_OUT_DIRECT(Obj::to_str(response.as_simple_string()));
                                        return true;
                                      });
                              if(!result)
                                throw fError("an error occurred while evaluating query %s",
                                             args->arg("message")->toString().c_str());
                              Terminal::singleton()->STD_OUT_DIRECT(Obj::to_str("\n"));
                              return model_obj;
                            } catch(const ollama::exception &e) {
                              throw fError("!yan ollama !rexception !yoccurred!!: %s", e.what());
                            } catch(const nlohmann::detail::exception &e) {
                              throw fError("!yan ollama !rjson exception !yoccurred!!: %s", e.what());
                            }
                          })
                          ->create()}});
              })
              ->create());
    }

    void setup() override {
      const ID server_url = this->rec_get("config/rest")->uri_value().toString();
      this->obj_set(
          "model",
          Helper::transform_vector<std::string>(Ollama(server_url.toString()).list_models(), [this](const string &m) {
            const auto model_obj = Obj::to_rec({{"name", vri(m)}, {"server", vri(*this->vid)}}, id_p(LLM_MODEL_TID));
            return model_obj;
          }));
    }
  };
} // namespace fhatos
#endif
#endif