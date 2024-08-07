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

namespace fhatos {

  class Rooter : public IDed, public Typed {
  protected:
    MutexDeque<Structure *> structures = MutexDeque<Structure *>();

  public:
    Rooter(const ID &id, const Pattern &pattern) : IDed(share(id)), Typed(share(pattern)) {}

    static Rooter *singleton(const ID &id = "/sys/rooter", const Pattern &pattern = "#") {
      static Rooter rooter = Rooter(id, pattern);
      return &rooter;
    }

    virtual void attach(Structure *structure) {
      this->structures.forEach([structure](Structure *s) {
        if (structure->type()->matches(*s->type()) || s->type()->matches(*structure->type())) {
          throw fError("Only !ydisjoint structures!! can coexist: !g[!y%m!g]!! is within !g[!m%s!g]!!\n",
                       s->type()->toString().c_str(), s->type()->toString().c_str());
        }
      });
      LOG_TASK(INFO, this, "attached structure %s\n", structure->type()->toString().c_str());
      this->structures.push_back(structure);
    }

    virtual void detach(const Pattern_p &structurePattern) {
      this->structures.remove_if([structurePattern](Structure *structure) {
        if (structure->type()->matches(*structurePattern)) {
          structure->close();
          // LOG_TASK(fhatos::LOG_TYPE::INFO, this, "detaching structure %s\n", structure->type()->toString().c_str());
          return true;
        }
        return false;
      });
    }

    bool publish(const Message_p &message) {
      this->structures.forEach([message](Structure *structure) {
        if (message->target.matches(*structure->type())) {
          structure->write(message);
        }
      });
    }

    bool subscribe(const Subscription_p &subscription) {
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
