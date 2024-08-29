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
#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include <structure/structure.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>
#include <util/ptr_helper.hpp>

namespace fhatos {

  class Router final : public Patterned {
  protected:
    MutexRW<> structures_mutex_ = MutexRW<>("<router structurers mutex>");
    ptr<Map<Pattern_p, Structure_p, furi_p_less>> structures_ = share(Map<Pattern_p, Structure_p, furi_p_less>());

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

    void stop() { LOG_STRUCTURE(INFO, this, "Stopping router %s\n", this->pattern()->toString().c_str()); }

    virtual void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_STRUCTURE(INFO, this, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype).c_str());
      } else {
        this->structures_mutex_.write<void *>([this, structure]() {
          for (const auto &[pt, st]: *this->structures_) {
            if (structure->pattern()->matches(*st->pattern()) || st->pattern()->matches(*structure->pattern())) {
              throw fError("Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!\n",
                           st->pattern()->toString().c_str(), structure->pattern()->toString().c_str());
            }
          }
          LOG_STRUCTURE(INFO, this, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                        StructureTypes.toChars(structure->stype).c_str());
          this->structures_->insert({structure->pattern(), structure});
          return nullptr;
        });
      }
    }

    virtual void detach(const Pattern_p &pattern) {
      this->structures_mutex_.write<void *>([this, pattern]() {
        List<Pattern_p> toRemove;
        for (const auto &pair: *this->structures_) {
          if (pair.first->matches(*pattern))
            toRemove.push_back(pair.first);
        }
        for (const auto &p: toRemove) {
          this->structures_->erase(p);
        }
        return nullptr;
      });
    }

    Objs_p read(const fURI_p &furi, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      LOG_STRUCTURE(TRACE, this, "reading !b%s!! for " FURI_WRAP "\n", furi->toString().c_str(),
                    source->toString().c_str());
      for (const auto &[pt, st]: *this->structures_) {
        if (furi->matches(*pt)) {
          return st->read(furi, source);
        }
      }
      throw fError(FURI_WRAP " has no structure to contain !b%s!!\n", this->pattern()->toString().c_str(),
                   furi->toString().c_str());
    }

    RESPONSE_CODE write(const ID_p &id, const Obj_p &obj, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      return this->route_message(share(Message{.source = *source, .target = *id, .payload = obj, .retain = true}));
    }

    void remove(const ID_p &id, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) {
      bool found = false;
      for (const auto &[pt, st]: *this->structures_) {
        if (id->matches(*pt)) {
          st->remove(id, source);
          found = true;
        }
      }
      if (!found)
        throw fError(FURI_WRAP " has no structure supporting !b%s!!\n", this->pattern()->toString().c_str(),
                     id->toString().c_str());
    }

    RESPONSE_CODE route_message(const Message_p &message) {
      bool found = false;
      for (const auto &[pt, st]: *this->structures_) {
        if (message->target.matches(*pt)) {
          st->recv_message(message);
          found = true;
        }
      }
      return found ? OK : NO_TARGETS;
    }

    RESPONSE_CODE route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) {
      bool found = false;
      for (const auto &[pt, st]: *this->structures_) {
        if (pattern->matches(*pt)) {
          st->recv_unsubscribe(subscriber, pattern);
          found = true;
        }
      }
      return found ? OK : NO_SUBSCRIPTION;
    }

    RESPONSE_CODE route_subscription(const Subscription_p &subscription) {
      bool found = false;
      for (const auto &[pt, st]: *this->structures_) {
        if (subscription->pattern.matches(*pt)) {
          st->recv_subscription(subscription);
          found = true;
        }
      }
      return found ? REPEAT_SUBSCRIPTION : OK;
    }
  };

  inline ptr<Router> router() { return Options::singleton()->router<Router>(); }
} // namespace fhatos

#endif
