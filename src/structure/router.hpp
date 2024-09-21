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
#include <util/mutex_rw.hpp>

namespace fhatos {
  class Router final : public Patterned {
    friend class System;

  protected:
    MutexRW<> structures_mutex_ = MutexRW<>("<router structures mutex>");
    ptr<Map<Pattern_p, Structure_p, furi_p_less>> structures_ = share(Map<Pattern_p, Structure_p, furi_p_less>());

    explicit Router(const Pattern &pattern): Patterned(p_p(pattern)) {
      LOG_ROUTER(INFO, "!yrouter!! started\n");
    }

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/") {
      static auto router_p = ptr<Router>(new Router(pattern));
      static bool setup = false;
      if (!setup)
        setup = true;
      return router_p;
    }

    void stop() {
      //   EPHEMERAL, VARIABLES, HARDWARE, DATABASE, DISTRIBUTED
      auto *ephemeral_count = new atomic_int(0);
      auto *variables_count = new atomic_int(0);
      auto *database_count = new atomic_int(0);
      auto *hardware_count = new atomic_int(0);
      auto *networked_count = new atomic_int(0);
      this->structures_mutex_.read<void *>(
        [this, ephemeral_count, variables_count, hardware_count,database_count,networked_count]() {
          for (const auto &pair: *this->structures_) {
            switch (pair.second->stype) {
              case SType::EPHEMERAL:
                ephemeral_count->fetch_add(1);
                break;
              case SType::VARIABLES:
                variables_count->fetch_add(1);
                break;
              case SType::DATABASE:
                database_count->fetch_add(1);
                break;
              case SType::HARDWARE:
                hardware_count->fetch_add(1);
                break;
              case SType::NETWORKED:
                networked_count->fetch_add(1);
                break;
            }
          }
          return nullptr;
        });
      LOG_ROUTER(
        INFO,
        "!yStopping!g %i !yephemeral!! | !g%i !yvariables!! | !g%i !ydatabase!! | !g%i !yhardware!! | !g%i !ynetworked!!\n",
        ephemeral_count->load(),
        variables_count->load(),
        database_count->load(),
        hardware_count->load(),
        networked_count->load());
      delete ephemeral_count;
      delete variables_count;
      delete database_count;
      delete hardware_count;
      delete networked_count;
      this->detach(p_p("#"));
      LOG_ROUTER(INFO, "!yrouter !b%s!! stopped\n", this->pattern()->toString().c_str());
    }

    void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_ROUTER(INFO, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                   StructureTypes.to_chars(structure->stype).c_str());
      } else {
        this->structures_mutex_.write<void *>([this, structure]() {
          for (const auto &pair: *this->structures_) {
            if (structure->pattern()->matches(*pair.second->pattern()) ||
                pair.second->pattern()->matches(*structure->pattern())) {
              // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
              throw fError(
                ROUTER_FURI_WRAP " Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                this->pattern_->toString().c_str(),
                pair.second->pattern()->toString().c_str(), structure->pattern()->toString().c_str());
            }
          }
          LOG_ROUTER(INFO, FURI_WRAP " !y%s!! attached\n", structure->pattern()->toString().c_str(),
                     StructureTypes.to_chars(structure->stype).c_str());
          this->structures_->insert({structure->pattern(), structure});
          return nullptr;
        });
      }
    }

    void detach(const Pattern_p &pattern) {
      this->structures_mutex_.write<void *>([this, pattern]() {
        List<Structure_p> toRemove;
        for (const auto &pair: *this->structures_) {
          if (pair.second->pattern()->matches(*pattern))
            toRemove.push_back(pair.second);
        }
        for (const Structure_p &structure: toRemove) {
          this->structures_->erase(structure->pattern());
          LOG_ROUTER(INFO, FURI_WRAP " !y%s!! detached\n",
                     structure->pattern()->toString().c_str(), StructureTypes.to_chars(structure->stype).c_str());
        }
        return nullptr;
      });
    }

    [[nodiscard]] Objs_p read(const fURI_p &furi) const {
      const Structure_p &struc = this->get_structure(p_p(*furi));
      LOG_ROUTER(DEBUG, "!y!_reading!! !b%s!! from " FURI_WRAP "\n", furi->toString().c_str(),
                 struc->pattern()->toString().c_str());
      return struc->read(furi);
    }

    void write(
      const ID_p &id, const Obj_p &obj,
      const bool retain = RETAIN_MESSAGE) const {
      const Structure_p &struc = this->get_structure(p_p(*id));
      LOG_ROUTER(DEBUG, "!y!_writing!! !g%s!! %s to !b%s!! at " FURI_WRAP "\n",
                 retain ? "retained" : "transient", obj->toString().c_str(),
                 id->toString().c_str(), struc->pattern()->toString().c_str());
      struc->write(id, obj, retain);
      MESSAGE_INTERCEPT(*id, obj, retain);
    }

    void remove(const ID_p &id) const {
      const Structure_p &struc = this->get_structure(p_p(*id));
      LOG_ROUTER(DEBUG, "!y!_removing!! !b%s!! from " FURI_WRAP "\n", id->toString().c_str(),
                 struc->pattern()->toString().c_str());
      struc->remove(id);
    }

    void route_message(const Message_p &message) const {
      const Structure_p &struc = this->get_structure(p_p(message->target));
      LOG_ROUTER(DEBUG, "!y!_routing message!! %s\n", message->toString().c_str());
      struc->recv_publication(message);
    }

    void route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) const {
      for (const auto &pair: *this->structures_) {
        if (pair.second->pattern()->matches(*pattern) || pattern->matches(*pair.second->pattern())) {
          LOG_ROUTER(DEBUG, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                     subscriber->toString().c_str());
          pair.second->recv_unsubscribe(subscriber, pattern);
        }
      }
    }

    void route_subscription(const Subscription_p &subscription) const {
      const Structure_p &struc = this->get_structure(p_p(subscription->pattern));
      LOG_ROUTER(DEBUG, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    }

  private:
    [[nodiscard]] Structure_p &get_structure(const Pattern_p &pattern) const {
      Structure_p *ret = nullptr;
      const Pattern_p temp = pattern->is_branch() ? p_p(pattern->extend("+")) : pattern;
      for (auto &pair: *this->structures_) {
        if (temp->matches(*pair.second->pattern())) {
          /// TODO: should be flipped?
          if (ret != nullptr)
            throw fError(ROUTER_FURI_WRAP " too general as it crosses multiple structures",
                         pattern->toString().c_str());
          ret = &pair.second;
        }
      }
      if (nullptr == ret)
        throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->pattern()->toString().c_str(),
                     pattern->toString().c_str());
      return *ret;
    }
  };

  inline ptr<Router> router() {
    return Options::singleton()->router<Router>();
  }
} // namespace fhatos

#endif
