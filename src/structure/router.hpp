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
#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include <structure/structure.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>
#include <util/ptr_helper.hpp>

namespace fhatos {

  class Router final : public Patterned {
  protected:
    MutexDeque<ptr<Structure>> structures = MutexDeque<ptr<Structure>>();
    explicit Router(const Pattern &pattern) : Patterned(p_p(pattern)) {}

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/") {
      static auto router_p = ptr<Router>(new Router(pattern));
      static bool _setup = false;
      if (!_setup) {
        _setup = true;
        LOG_STRUCTURE(INFO, router_p.get(), "!yrouter!! loaded\n");
      }
      return router_p;
    }

    virtual void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_STRUCTURE(INFO, this, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype).c_str());
      } else {
        this->structures.forEach([structure](const ptr<Structure> &s) {
          if (structure->pattern()->matches(*s->pattern()) || s->pattern()->matches(*structure->pattern())) {
            throw fError("Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!\n",
                         s->pattern()->toString().c_str(), structure->pattern()->toString().c_str());
          }
        });
        // structure->setup();
        LOG_STRUCTURE(INFO, this, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype).c_str());
        this->structures.push_back(structure);
      }
    }

    virtual void detach(const Pattern_p &structurePattern) {
      this->structures.remove_if([this, structurePattern](const ptr<Structure> &structure) {
        if (structure->pattern()->matches(*structurePattern)) {
          structure->stop();
          LOG_STRUCTURE(INFO, this, "detached structure %s\n", structure->pattern()->toString().c_str());
          // delete structure;
          return true;
        }
        return false;
      });
    }

    Objs_p read(const fURI_p &furi, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      if (furi->is_pattern()) {
        LOG_STRUCTURE(TRACE, this, "reading !b%s!! for " FURI_WRAP "\n", furi->toString().c_str(),
                      source->toString().c_str());
        auto *s = new atomic<Structure *>(nullptr);
        this->structures.forEach([furi, s, source](const ptr<Structure> &structure) {
          if (furi->matches(*structure->pattern())) {
            s->store(structure.get());
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
      } else {
        LOG_STRUCTURE(TRACE, this, "reading !b%s!! for " FURI_WRAP "\n", furi->toString().c_str(),
                      source->toString().c_str());
        auto *s = new atomic<Structure *>(nullptr);
        this->structures.forEach([furi, s, source](const ptr<Structure> &structure) {
          if (furi->matches(*structure->pattern())) {
            s->store(structure.get());
          }
        });
        if (!s->load()) {
          delete s;
          throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                       furi->toString().c_str());
        }
        Obj_p ret = s->load()->read(furi, source);
        delete s;
        return share(Obj(any(ret->_value), id_p(*ret->id()))); // why is a copy required?
      }
    }


    void write(const ID_p &id, const Obj_p &obj, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      this->route_message(share(Message{.source = *source, .target = *id, .payload = obj, .retain = true}));
    }

    void remove(const ID_p &id, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      auto *found = new atomic_bool(false);
      this->structures.forEach([found, id, source](const ptr<Structure> &structure) {
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
      this->structures.forEach([message, rc](const ptr<Structure> &structure) {
        if (message->target.matches(*structure->pattern())) {
          structure->recv_message(message);
          *rc = OK;
        }
      });
      const auto rc2 = RESPONSE_CODE(*rc);
      LOG(DEBUG, "[!r%s!!] " FURI_WRAP " !yrouted message %s\n", ResponseCodes.toChars(rc2).c_str(),
          this->_pattern->toString().c_str(), message->toString().c_str());
      delete rc;
      return rc2;
    }

    RESPONSE_CODE route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) {
      auto *rc = new RESPONSE_CODE(NO_TARGETS);
      this->structures.forEach([subscriber, pattern, rc](const ptr<Structure> &structure) {
        if (pattern->matches(*structure->pattern())) {
          structure->recv_unsubscribe(subscriber, p_p(*pattern));
          *rc = OK;
        }
      });
      auto rc2 = RESPONSE_CODE(*rc);
      LOG(DEBUG, "[!r%s!!] " FURI_WRAP " !yrouted!! !_!yun!!!ysubscription!! " FURI_WRAP "=unsubscribe=>!y%s!!\n",
          ResponseCodes.toChars(rc2).c_str(), this->pattern()->toString().c_str(), subscriber->toString().c_str(),
          pattern->toString().c_str());
      // delete rc;
      return rc2;
    }

    RESPONSE_CODE route_subscription(const Subscription_p &subscription) {
      auto *rc = new RESPONSE_CODE(NO_TARGETS);
      this->structures.forEach([subscription, rc](const ptr<Structure> &structure) {
        if (subscription->pattern.matches(*structure->pattern())) {
          structure->recv_subscription(subscription);
          *rc = OK;
        }
      });
      auto rc2 = RESPONSE_CODE(*rc);
      LOG(DEBUG, "[!r%s!!] " FURI_WRAP " !yrouted subscription!! %s\n", ResponseCodes.toChars(rc2).c_str(),
          this->pattern()->toString().c_str(), subscription->toString().c_str());
      // delete rc;
      return rc2;
    }
  };

  ptr<Router> router() { return Options::singleton()->router<Router>(); }
} // namespace fhatos

#endif
