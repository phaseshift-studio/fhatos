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

#include "../../../fhatos.hpp"
#include "../../../furi.hpp"
#include "../sys/router/structure.hpp"

#define FRAME_TID  "/sys/router/frame"

namespace fhatos {

  template<typename ALLOCATOR = std::allocator<std::pair<const ID_p, Obj_p>>>
  class Frame final : public Structure {
  protected:
    Rec_p data_;

  public:
    ptr<Frame<>> previous_{};

    explicit Frame(const Pattern &pattern,
                   const ptr<Frame> &previous = nullptr,
                   const Rec_p &frame_data = Obj::to_rec()) :
      Structure(pattern, id_p(FRAME_TID)), //id_p(pattern.retract())),
      data_{frame_data->rec_no_query()}, previous_{previous} {
    }


    [[nodiscard]] size_t depth() const {
      return this->previous_ ? this->previous_->depth() + 1 : 1;
    }

    [[nodiscard]] Rec_p full_frame() const {
      const Rec_p all_frames = this->previous_ ? this->previous_->full_frame() : Obj::to_rec();
      for(const auto &[key,value]: *this->data_->rec_value()) {
        all_frames->rec_value()->insert({key, value});
      }
      return all_frames;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    Obj_p read(const fURI &furi) override {
      const fURI furi_no_query = furi.no_query();
      if(this->previous_ && StringHelper::is_integer(furi_no_query.toString())) { // unnamed args accessed by index
        const int index = stoi(furi_no_query.toString());
        if(index >= this->previous_->data_->rec_value()->size())
          return Obj::to_noobj();
        auto itty = this->previous_->data_->rec_value()->begin();
        std::advance(itty, index);
        return itty->second;
      }
      return !furi_no_query.matches(*this->pattern) || this->data_->rec_value()->count(vri(furi_no_query)) == 0
               ? (nullptr == this->previous_ ? nullptr : this->previous_->read(furi_no_query))
               : this->data_->rec_value()->at(vri(furi_no_query));
    }

    void write(const fURI &furi, const Obj_p &obj, const bool retain) override {
      const fURI furi_no_query = furi.no_query();
      if(this->data_->rec_value()->count(vri(furi_no_query)))
        throw fError("frame structures objs are read-only");
      if(this->previous_)
        this->previous_->write(furi_no_query, obj, retain);
      else
        ROUTER_WRITE(furi, obj, retain);
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
