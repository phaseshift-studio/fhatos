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
#ifndef fhatos_base_fs_hpp
#define fhatos_base_fs_hpp

#include "../../../fhatos.hpp"
#include "../../structure.hpp"
#include "../../../util/obj_helper.hpp"

namespace fhatos {

  const static ID FS_FURI = ID("/io/lib/fs");

  class BaseFS : public Structure {

  protected:
    ID root;

    explicit BaseFS(const Pattern &pattern, const ID_p &value_id = nullptr,
                    const Rec_p &config = Obj::to_rec({{"root", vri(".")}})) :
      Structure(pattern, id_p(FS_FURI), value_id, config), root(config->rec_get("root")->uri_value()) {
    }

    ID map_fos_to_fs(const ID_p &fos_id) const {
      auto fs_id = ID(*fos_id);
      const fURI fs_retracted_id = fs_id.remove_subpath(this->pattern->retract_pattern().toString());
     // LOG(INFO, "current pretracted pattern: %s\n", fs_retracted_id.toString().c_str());
     // LOG(INFO, "current extended root: %s\n", this->root.extend(fs_retracted_id).toString().c_str());
      return this->root.extend(fs_retracted_id);
    }

    ID map_fs_to_fos(const string &fs_id) const {
      const auto fos_id = ID(fs_id);
      const fURI fos_retracted_id = fos_id.remove_subpath(this->root.toString());
      const fURI retracted_pattern = this->pattern->retract_pattern();
      return retracted_pattern.extend(fos_retracted_id);
    }

  public:
    template<typename STRUCTURE>
    static void *import(const ID &import_id) {
      static_assert(std::is_base_of_v<BaseFS, STRUCTURE>, "STRUCTURE should be derived from BaseMqtt");
      Router::import_structure<STRUCTURE>(import_id, FS_FURI);
      return nullptr;
    }
  };
} // namespace fhatos
#endif
