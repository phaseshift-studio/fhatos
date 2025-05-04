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
#ifndef fhatos_fs_hpp
#define fhatos_fs_hpp

#include "../../../../fhatos.hpp"
#include "../../sys/router/router.hpp"
#include "../../sys/router/structure/structure.hpp"

namespace fhatos {

  const static auto FS_FURI = ID("/sys/lib/fs");

  class FS : public Structure {

  protected:
    ID root;

    void write_raw_pairs(const ID &id, const Obj_p &obj, bool retain) override;

    IdObjPairs read_raw_pairs(const fURI &match) override;

  public:
    explicit FS(const Pattern &pattern, const ID_p &value_id = nullptr,
                const Rec_p &config = Obj::to_rec({{"root", vri(".")}})); /* :
   Structure(pattern, id_p(FS_FURI), value_id, config), root(config->rec_get("root")->uri_value()) {
 }*/

    static ptr<FS> create(const Pattern &pattern, const ID_p &value_id = nullptr, const Rec_p &config = Obj::to_rec()) {
      return Structure::create<FS>(pattern, value_id, config);
    }

    static Obj_p load_boot_config(const fURI &boot_config = FOS_BOOT_CONFIG_FS_URI);

    ID map_fos_to_fs(const ID &fos_id) const {
      const fURI fs_retracted_id = fos_id.remove_subpath(this->pattern->retract_pattern().toString());
      return this->root.extend(fs_retracted_id);
    }

    ID map_fs_to_fos(const string &fs_id) const {
      const auto fos_id = ID(fs_id);
      const fURI fos_retracted_id = fos_id.remove_subpath(this->root.toString());
      const fURI retracted_pattern = this->pattern->retract_pattern();
      return retracted_pattern.extend(fos_retracted_id);
    }

    static void *import(const ID &import_id) {
      Router::import_structure<FS>(import_id, FS_FURI);
      return nullptr;
    }
  };
} // namespace fhatos
#endif
