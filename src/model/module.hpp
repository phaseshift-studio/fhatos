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
#ifndef fhatos_modules_hpp
#define fhatos_modules_hpp

#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../lang/obj.hpp"
#include "fos/sys/typer/typer.hpp"

namespace fhatos {
  using ModuleTypes = std::vector<std::pair<ID, Obj_p>>;


  class Module final : public Rec {
  protected:
    uptr<Map<fURI, Supplier<ModuleTypes>>> MODULES = make_unique<Map<fURI, Supplier<ModuleTypes>>>();

  public:
    explicit Module(const ID &id = "/sys/module") : Rec(Obj(rmap({}), OType::REC, REC_FURI, id_p(id))) {}

    static ptr<Module> singleton(const ID &id = "/sys/module") {
      static auto modules = make_shared<Module>(id);
      return modules;
    }

    void register_module(const fURI &pattern, const Supplier<ModuleTypes> &type_generator) {
      MODULES->insert_or_assign(pattern, type_generator);
    }

    void import(const fURI &pattern) {
      for(const auto &[k, v]: *MODULES) {
        const ModuleTypes types = v();
        if(k.bimatches(pattern)) {
          Typer::singleton()->start_progress_bar(types.size());
          for(const auto &[id, type_def]: types) {
            if(id.matches(pattern)) {
              Typer::singleton()->save_type(id, type_def);
            }
            Typer::singleton()->end_progress_bar(
                StringHelper::format("\n\t\t!^u1^ !g[!b%s !ytypes!! loaded!g]!! \n", k.toString().c_str()));
          }
        }
      }
    }
  };
} // namespace fhatos
#endif
