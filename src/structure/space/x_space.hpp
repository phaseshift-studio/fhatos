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
#ifndef fhatos_space_hpp
#define fhatos_space_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/router/router.hpp>
#include <util/pubsub_manager.hpp>

namespace fhatos {

  class XSpace : public IDed {

  protected:
    const Pattern_p _range;
    const unique_ptr<PubSubManager> manager_;

  public:
    XSpace(const ID_p &id, const Pattern_p &range, const PubSubManager &m = PubSubManager(true)) :
        IDed(id), _range(range), manager_(std::make_unique<PubSubManager>(&m)) {}

    const PubSubManager *pubsub() const { return manager_.get(); }

    bool in_range(const Pattern &pattern) const { return this->_range->matches(pattern); }

    const Pattern_p range() const { return this->_range; }

    virtual Obj_p find(const fURI &furi) const = 0;
  };

  struct SpaceBuilder {
    const ID_p id_;
    Pattern_p range_;
    XSpace *space_;

    SpaceBuilder(const ID_p &id) : id_(id) {}

    static SpaceBuilder *start(const ID_p &id) { return new SpaceBuilder(id); }


    SpaceBuilder *range(const Pattern_p &range) {
      this->range_ = range;
      return this;
    }

    SpaceBuilder *space(XSpace *space) {
      this->space_ = space;
      return this;
    }

    // ptr<XSpace> create() {
    //   return share(XSpace(this->id_,this->range_));
    // }
  };
} // namespace fhatos

#endif
