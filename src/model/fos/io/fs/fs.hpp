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

#define FS_TID "/fos/s/fs"

namespace fhatos {

  class FS final : public Structure {

  protected:
    ID root;

    void write_raw_pairs(const ID &id, const Obj_p &obj, bool retain) override;

    IdObjPairs read_raw_pairs(const fURI &match) override;

  public:
    explicit FS(const Pattern &span, const ID_p &vid = nullptr,
                const Rec_p &config = Obj::to_rec({{"root", vri(".")}}));

    static ptr<FS> create(const Pattern &span, const ID_p &vid = nullptr, const Rec_p &config = Obj::to_rec());

    static Obj_p load_boot_config(const fURI &boot_config = FOS_BOOT_CONFIG_FS_URI);

    static void list_files_utility(const char *path);

    void setup() override;

    void stop() override { Structure::stop(); }

    ID map_fos_to_fs(const ID &fos_id) const;

    ID map_fs_to_fos(const string &fs_id) const;

    static void *import() {
      Router::import_structure<FS>(FOS_URI "/s/fs");
      return nullptr;
    }
  };
} // namespace fhatos
#endif
