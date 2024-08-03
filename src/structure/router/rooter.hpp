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
#ifndef fhatos_rooter_hpp
#define fhatos_rooter_hpp

#include <structure/space/x_space.hpp>

namespace fhatos {

  class Rooter {
  protected:
    List<const XSpace *> spaces = List<const XSpace *>();

  public:
    void registerSpace(XSpace *space) {
      for (const auto *s: this->spaces) {
        if (s->range()->matches(*space->range()) || space->range()->matches(*s->range())) {
          throw fError("Only !ydisjoint spaces!! can be registered: !b%s!! !g[!y%s!g]!! within !b%s!! !g[!y%s!g]!!\n", s->id()->toString().c_str(),
                       s->range()->toString().c_str(), space->id()->toString().c_str(),
                       space->range()->toString().c_str());
        }
      }
      this->spaces.push_back(space);
    }
    void unregisterSpace(const ID_p &spaceId) {
      // erase_if(this->spaces, [spaceId](const XSpace &space) { return true; });
    }

    bool route(const Message_p &message) {
      for (auto *space: spaces) {
        if (space->in_range(Pattern(message->target))) {
          // space->publish(message);
        }
      }
    }

    bool route(const Subscription_p &subscription) {
      for (const auto *space: spaces) {
        if (space->in_range(subscription->pattern)) {
          // space->pubsub()->subscribe(*subscription);
        }
      }
      return true;
    }
  };
} // namespace fhatos

#endif
