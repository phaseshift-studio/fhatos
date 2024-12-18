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

#ifndef fhatos_frame_hpp
#define fhatos_frame_hpp

#include "../../fhatos.hpp"
#include "../../furi.hpp"
#include "../structure.hpp"

namespace fhatos {
  template<typename ALLOCATOR = std::allocator<std::pair<const ID_p, Obj_p>>>
  class Frame final : public Structure {
  public:
    ptr<Frame> previous;

    const unique_ptr<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>> data_ =
        make_unique<Map<const ID_p, Obj_p, furi_p_less, ALLOCATOR>>();

    explicit Frame(const Pattern &pattern,
                   const ptr<Frame> &previous = nullptr) : Structure(pattern, ID(pattern.retract())),
                                                           previous{previous} {
    }

  public:
    Obj_p read(const fURI_p &furi) override {
      const ID_p id = id_p(*furi);
      return !furi->matches(*this->pattern_) || this->data_->count(id) == 0
               ? (nullptr == this->previous ? nullptr : this->previous->read(furi))
               : this->data_->at(id);
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain) override {
      if(const ID_p &id = id_p(*furi); this->data_->count(id))
        throw fError("frame structures objs are read-only");
      if(this->previous) {
        this->previous->write(furi, obj, retain);
      } else
        ROUTER_WRITE(furi, obj, retain);
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, bool retain) override {
      throw fError("Frame::write_raw_pairs is unreachable code");
    }

    IdObjPairs read_raw_pairs(const fURI_p &match) override {
      if(true)
        throw fError("Frame::read_raw_pairs is unreachable code");
      return IdObjPairs();
    }
  };
} // namespace fhatos

#endif
