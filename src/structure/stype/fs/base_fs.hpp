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
  class BaseFS : public Structure {

  private:
    explicit BaseFS(const Rec_p &rec) : Structure(rec), root{ID(rec->rec_get("config/root")->uri_value())} {  
    }

  protected:
    ID root;
    explicit BaseFS(const Pattern &pattern,const Rec_p &config) : BaseFS(Obj::to_rec({{"pattern", vri(pattern)},{FOS_CONFIG, config}}, REC_FURI)) {
    }

    ID map_fos_to_fs(const ID_p& fos_id) {
      ID fs_id = ID(*fos_id);
      for(uint8_t i =0; i<this->pattern()->path_length();i++) {
        fs_id = fs_id.pretract();
      }
      return root.extend(fs_id);
    }

    ID map_fs_to_fos(const char* fs_id) {
      ID fos_id = ID(fs_id);
      for(uint8_t i =0; i<root.path_length();i++) {
        fos_id = fos_id.pretract();
      }
      return fos_id;
    }
  };
} // namespace fhatos
#endif
