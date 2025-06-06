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
#ifndef fhatos_llm_hpp
#define fhatos_llm_hpp

#include "../../../../fhatos.hpp"
#include "../../../../lang/util/ollama.hpp"

namespace fhatos {

  class LLM final : Rec {

  public:
    LLM(const Rec_p &llm_obj) : Rec(*llm_obj) {}

    static ptr<LLM> create(
        const ID_p &vid = nullptr,
        const Rec_p &config = Obj::to_rec({{"model", Obj::to_uri("ALIENTELLIGENCE/embeddedsystemsengineer:latest")}})) {
      const auto llm = make_shared<LLM>(Obj::to_rec({{"config", config}}, id_p("/fos/llm"), vid));
      InstBuilder::build(llm->vid->add_component("query"))
          ->inst_args("message?str", __())
          ->inst_f([](const Obj_p &llm, const InstArgs &args) {
            return Obj::to_str(((LLM *) llm.get())->query(args->arg("message")->str_value()));
          })
          ->save();
      return llm;
    }

    string query(const string &message) {
      try {
        Ollama ollama_server = Ollama("http://fhat.duckdns.org:11434");
        return ollama_server.generate(this->rec_get("config/model")->uri_value().toString(), message).as_simple_string();
      } catch(ollama::exception &e) {
        throw fError("ollama exception: %s", e.what());
      }
    }
  };
} // namespace fhatos
#endif
