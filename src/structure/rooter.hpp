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
    static Rooter *singleton(const Pattern &pattern = "/router/") {
      static Rooter rooter = Rooter(pattern);
      static bool _setup = false;
      if (!_setup) {
        _setup = true;
        LOG_STRUCTURE(INFO, &rooter, "!yrouter!! loaded\n");
      }
      return &rooter;
    }

    virtual void attach(Structure *structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_STRUCTURE(INFO, this, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype));
      } else {
        this->structures.forEach([structure](Structure *s) {
          if (structure->pattern()->matches(*s->pattern()) || s->pattern()->matches(*structure->pattern())) {
            throw fError("Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!\n",
                         s->pattern()->toString().c_str(), structure->pattern()->toString().c_str());
          }
        });
        // structure->setup();
        LOG_STRUCTURE(INFO, this, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype));
        this->structures.push_back(structure);
      }
    }

    virtual void detach(const Pattern_p &structurePattern) {
      this->structures.remove_if([this, structurePattern](Structure *structure) {
        if (structure->pattern()->matches(*structurePattern)) {
          structure->stop();
          LOG_STRUCTURE(INFO, this, "detached structure %s\n", structure->pattern()->toString().c_str());
          return true;
        }
        return false;
      });
    }

    Objs_p read(const fURI_p &furi, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      LOG_STRUCTURE(TRACE, this, "reading !b%s!! for " FURI_WRAP "\n", furi->toString().c_str(),
                    source->toString().c_str());
      auto *s = new atomic<Structure *>(nullptr);
      this->structures.forEach([furi, s, source](Structure *structure) {
        if (furi->matches(*structure->pattern())) {
          s->store(structure);
        }
      });
      if (!s->load()) {
        delete s;
        throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                     furi->toString().c_str());
      }
      Objs_p ret = s->load()->read(furi, source);
      delete s;
      return ret;
    }

    Obj_p read(const ID_p &id, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      LOG_STRUCTURE(TRACE, this, "reading !b%s!! for " FURI_WRAP "\n", id->toString().c_str(),
                    source->toString().c_str());
      auto *s = new atomic<Structure *>(nullptr);
      this->structures.forEach([id, s, source](Structure *structure) {
        if (id->matches(*structure->pattern())) {
          s->store(structure);
        }
      });
      if (!s->load()) {
        delete s;
        throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                     id->toString().c_str());
      }
      Obj_p ret = s->load()->read(id, source);
      delete s;
      return share(Obj(any(ret->_value), id_p(*ret->id()))); // why is a copy required?
    }

    void write(const ID_p &id, const Obj_p &obj, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      LOG_STRUCTURE(TRACE, this, "writing %s to !b%s!! for " FURI_WRAP "\n", obj->toString().c_str(),
                    id->toString().c_str(), source->toString().c_str());
      auto *found = new atomic_bool(false);
      this->structures.forEach([found, id, obj, source](Structure *structure) {
        if (!found->load()) {
          if (id->matches(*structure->pattern())) {
            structure->write(id, obj, source);
            found->store(true);
          }
        }
      });
      if (!found->load()) {
        delete found;
        throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                     id->toString().c_str());
      }
      delete found;
    }


    void remove(const ID_p &id, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      auto *found = new atomic_bool(false);
      this->structures.forEach([found, id, source](Structure *structure) {
        if (!found->load()) {
          if (id->matches(*structure->pattern())) {
            structure->remove(id, source);
            found->store(true);
          }
        }
      });
      if (!found->load()) {
        delete found;
        throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                     id->toString().c_str());
      }
      delete found;
    }


    RESPONSE_CODE route_message(const Message_p &message) {
      auto *rc = new RESPONSE_CODE(NO_SUBSCRIPTION);
      this->structures.forEach([message, rc](Structure *structure) {
        if (message->target.matches(*structure->pattern())) {
          structure->recv_message(message);
          *rc = OK;
        }
      });
      auto rc2 = RESPONSE_CODE(*rc);
      LOG(TRACE, "!r%s!! for !yrouted message!! %s\n", ResponseCodes.toChars(rc2), message->toString().c_str());
      delete rc;
      return rc2;
    }

    RESPONSE_CODE route_subscription(const Subscription_p &subscription) {
      auto *rc = new RESPONSE_CODE(NO_TARGETS);
      this->structures.forEach([subscription, rc](Structure *structure) {
        if (subscription->pattern.matches(*structure->pattern())) {
          structure->recv_subscription(subscription);
          *rc = OK;
        }
      });
      auto rc2 = RESPONSE_CODE(*rc);
      LOG(TRACE, "!r%s!! for !yrouted subscription!! %s\n", ResponseCodes.toChars(rc2),
          subscription->toString().c_str());
      delete rc;
      return rc2;
    }
  };
} // namespace fhatos

#endif
