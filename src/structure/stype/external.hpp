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

#ifndef fhatos_external_hpp
#define fhatos_external_hpp

#include <fhatos.hpp>
#include <structure/structure.hpp>
#include <util/pubsub_helper.hpp>
#include <language/obj.hpp>
#include <furi.hpp>

namespace fhatos {
  class External : public KeyValue {
  protected:
    Map<Pattern, Function<fURI_p, Obj_p>, furi_p_less> data_map_;

    explicit External(
      const Pattern &pattern = "+",
      const Map<Pattern, Function<fURI_p, Obj_p>, furi_p_less> &data_map = {}) : KeyValue(pattern, SType::HARDWARE),
      data_map_(data_map) {
    }

    // CALL THIS DYNAMIC
    /*virtual Obj_p read(const fURI_p& target) override {
         for (const auto &[pattern,con]: this->data_map_) {
           if (target->matches(pattern))
             return con(target);
         }
         return noobj();
    }

    virtual void remove(const ID_p& target) override {
     if(this->data_map_.count(Pattern(*target)))
        this->data_map_.erase(Pattern(*target));
    }*/
  };
} // namespace fhatos

#endif
