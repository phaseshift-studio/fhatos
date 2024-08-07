//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#ifndef fhatos_rooter_hpp
#define fhatos_rooter_hpp

#include <structure/structure.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>

namespace fhatos {

  class Rooter : public Patterned {
  protected:
    MutexDeque<Structure *> structures = MutexDeque<Structure *>();
    explicit Rooter(const Pattern &pattern) : Patterned(share(pattern)) {}

  public:
    static Rooter *singleton(const Pattern &pattern = "#") {
      static Rooter rooter = Rooter(pattern);
      return &rooter;
    }

    virtual void attach(Structure *structure) {
      this->structures.forEach([structure](Structure *s) {
        if (structure->type()->matches(*s->type()) || s->type()->matches(*structure->type())) {
          throw fError("Only !ydisjoint structures!! can coexist: !g[!y%m!g]!! is within !g[!m%s!g]!!\n",
                       s->type()->toString().c_str(), s->type()->toString().c_str());
        }
      });
      LOG_STRUCTURE(INFO, this, "attached structure %s\n", structure->type()->toString().c_str());
      this->structures.push_back(structure);
    }

    virtual void detach(const Pattern_p &structurePattern) {
      this->structures.remove_if([this, structurePattern](Structure *structure) {
        if (structure->type()->matches(*structurePattern)) {
          structure->close();
          LOG_STRUCTURE(INFO, this, "detaching structure %s\n", structure->type()->toString().c_str());
          return true;
        }
        return false;
      });
    }

    bool route_message(const Message_p &message) {
      this->structures.forEach([message](Structure *structure) {
        if (message->target.matches(*structure->type())) {
          structure->write(message);
        }
      });
    }

    bool route_subscription(const Subscription_p &subscription) {
      this->structures.forEach([subscription](Structure *structure) {
        if (subscription->pattern.matches(*structure->type())) {
          structure->read(subscription);
        }
      });
      return true;
    }
  };
} // namespace fhatos

#endif
