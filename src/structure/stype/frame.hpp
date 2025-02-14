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

  const ID FRAME_FURI = "/sys/router/frame";

  template<typename ALLOCATOR = std::allocator<std::pair<const ID_p, Obj_p>>>
  class Frame final : public Structure {
  protected:
    const unique_ptr<OrderedMap<const ID, Obj_p, furi_hash, furi_equal_to>> data_ =
        make_unique<OrderedMap<const ID, Obj_p, furi_hash, furi_equal_to>>();

  public:
    ptr<Frame<>> previous;

    explicit Frame(const Pattern &pattern,
                   const ptr<Frame> &previous = nullptr,
                   const Rec_p &frame_data = Obj::to_rec()) :
      Structure(pattern,id_p(FRAME_FURI)), //id_p(pattern.retract())),
      previous{previous} {
      for(const auto &[key,value]: *frame_data->rec_value()) {
        this->data_->insert({key->uri_value(), value});
      }
    }


    size_t depth() const {
      return this->previous ? this->previous->depth() + 1 : 1;
    }

    Rec_p full_frame() const {
      const Rec_p all_frames = this->previous ? this->previous->full_frame() : Obj::to_rec();
      for(const auto &[key,value]: *this->data_) {
        all_frames->rec_value()->insert({vri(key), value});
      }
      return all_frames;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    Obj_p read(const fURI &furi) override {
      if(this->previous && StringHelper::is_integer(furi.toString())) { // unnamed args accessed by index
        const int index = stoi(furi.toString());
        if(index >= this->previous->data_->size())
          return Obj::to_noobj();
        auto itty = this->previous->data_->begin();
        std::advance(itty, index);
        return itty->second;
      }
      return !furi.matches(*this->pattern) || this->data_->count(furi) == 0
               ? (nullptr == this->previous ? nullptr : this->previous->read(furi))
               : this->data_->at(furi);
    }

    void write(const fURI &furi, const Obj_p &obj, const bool retain) override {
      if(this->data_->count(furi))
        throw fError("frame structures objs are read-only");
      if(this->previous)
        this->previous->write(furi, obj, retain);
      else
        ROUTER_WRITE(id_p(furi), obj, retain);
    }

    void write_raw_pairs(const ID &, const Obj_p &, bool retain) override {
      throw fError("frame::write_raw_pairs is unreachable code");
    }

    IdObjPairs read_raw_pairs(const fURI &) override {
      throw fError("frame::read_raw_pairs is unreachable code");
    }
  };
} // namespace fhatos

#endif
