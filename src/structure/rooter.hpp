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
      LOG_STRUCTURE(INFO, this, "attached structure !b%s!!\n", structure->type()->toString().c_str());
      structure->setup();
      this->structures.push_back(structure);
    }

    virtual void detach(const Pattern_p &structurePattern) {
      this->structures.remove_if([this, structurePattern](Structure *structure) {
        if (structure->type()->matches(*structurePattern)) {
          structure->stop();
          LOG_STRUCTURE(INFO, this, "detached structure %s\n", structure->type()->toString().c_str());
          return true;
        }
        return false;
      });
    }

    RESPONSE_CODE route_message(const Message_p &message) {
      RESPONSE_CODE *rc = new RESPONSE_CODE(NO_SUBSCRIPTION);
      this->structures.forEach([message, rc](Structure *structure) {
        if (message->target.matches(*structure->type())) {
          structure->recv_message(message);
          *rc = OK;
        }
      });
      RESPONSE_CODE rc2 = RESPONSE_CODE(*rc);
      LOG(TRACE, "!r%s!! for !yrouted message!! %s\n", ResponseCodes.toChars(rc2), message->toString().c_str());
      delete rc;
      return rc2;
    }

    RESPONSE_CODE route_subscription(const Subscription_p &subscription) {
      RESPONSE_CODE *rc = new RESPONSE_CODE(NO_TARGETS);
      this->structures.forEach([subscription, rc](Structure *structure) {
        if (subscription->pattern.matches(*structure->type())) {
          structure->recv_subscription(subscription);
          *rc = OK;
        }
      });
      RESPONSE_CODE rc2 = RESPONSE_CODE(*rc);
      LOG(TRACE, "!r%s!! for !yrouted subscription!! %s\n", ResponseCodes.toChars(rc2),
          subscription->toString().c_str());
      delete rc;
      return rc2;
    }
  };
} // namespace fhatos

#endif
