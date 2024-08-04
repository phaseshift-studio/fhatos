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

  class XSpace {

  protected:
    const Pattern_p range_;
    const unique_ptr<PubSubManager> manager_;

  public:
    XSpace(const Pattern_p &range, const PubSubManager &m = PubSubManager(true)) :
        range_(range), manager_(std::make_unique<PubSubManager>(&m)) {}

    PubSubManager *pubsub() const { return manager_.get(); }

    bool in_range(const Pattern &pattern) const { return this->range_->matches(pattern); }

    const Pattern_p range() const { return this->range_; }

    virtual Obj_p find(const ID &id) const = 0;

    static fError ID_NOT_IN_RANGE(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! is not within the boundaries of space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }

    static fError ID_DOES_NOT_EXIST(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! does not reference an obj in space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }
  };
} // namespace fhatos

#endif
