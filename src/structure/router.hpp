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

#include <ranges>
#include <structure/structure.hpp>
#include <util/mutex_rw.hpp>

namespace fhatos {
  class Router final : public Patterned {
  protected:
    MutexRW<> structures_mutex_ = MutexRW<>("<router structures mutex>");
    ptr<Map<Pattern_p, Structure_p, furi_p_less>> structures_ = share(Map<Pattern_p, Structure_p, furi_p_less>());

    explicit Router(const Pattern &pattern) : Patterned(p_p(pattern)) {
    }

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/") {
      static auto router_p = ptr<Router>(new Router(pattern));
      static bool _setup = false;
      if (!_setup) {
        _setup = true;
        LOG_STRUCTURE(INFO, router_p.get(), "!yrouter!! !b%s!! loaded\n", pattern.toString().c_str());
      }
      return router_p;
    }

    void stop() {
      auto *readCount = new atomic_int(0);
      auto *writeCount = new atomic_int(0);
      auto *readWriteCount = new atomic_int(0);
      this->structures_mutex_.read<void *>([this,readCount,writeCount,readWriteCount]() {
        for (const Structure_p &structure: *this->structures_ | views::values) {
          switch (structure->stype) {
            case SType::READ:
              readCount->fetch_add(1);
              break;
            case SType::WRITE:
              writeCount->fetch_add(1);
              break;
            case SType::READWRITE:
              readWriteCount->fetch_add(1);
              break;
          }
        }
        return nullptr;
      });
      LOG_STRUCTURE(INFO, this, "!yStopping!g %i !yreads!! | !g%i !ywrites!! | !g%i !yreadwrites!!\n",
                    readCount->load(),
                    writeCount->load(),
                    readWriteCount->load());
      this->detach(p_p("#"));
      LOG_STRUCTURE(INFO, this, "!yrouter !b%s!! stopped\n", this->pattern()->toString().c_str());
    }

    void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_STRUCTURE(INFO, this, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                      StructureTypes.toChars(structure->stype).c_str());
      } else {
        this->structures_mutex_.write<void *>([this, structure]() {
          for (const Structure_p &struc: *this->structures_ | views::values) {
            if (structure->pattern()->matches(*struc->pattern()) || struc->pattern()->matches(*structure->pattern())) {
              throw fError("Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!\n",
                           struc->pattern()->toString().c_str(), structure->pattern()->toString().c_str());
            }
          }
          LOG_STRUCTURE(INFO, this, FURI_WRAP " !y%s!! attached\n", structure->pattern()->toString().c_str(),
                        StructureTypes.toChars(structure->stype).c_str());
          this->structures_->insert({structure->pattern(), structure});
          return nullptr;
        });
      }
    }

    void detach(const Pattern_p &pattern) {
      this->structures_mutex_.write<void *>([this, pattern]() {
        List<Structure_p> toRemove;
        for (const Structure_p &structure: *this->structures_ | views::values) {
          if (structure->pattern()->matches(*pattern))
            toRemove.push_back(structure);
        }
        for (const Structure_p &structure: toRemove) {
          this->structures_->erase(structure->pattern());
          LOG_STRUCTURE(INFO, this, FURI_WRAP " !y%s!! detached\n",
                        structure->pattern()->toString().c_str(), StructureTypes.toChars(structure->stype).c_str());
        }
        return nullptr;
      });
    }

    [[nodiscard]] Objs_p read(const fURI_p &furi, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) const {
      const Structure_p &struc = this->get_structure(p_p(*furi), source);
      LOG_STRUCTURE(DEBUG, this, "!y!_reading!! !b%s!! from " FURI_WRAP " for %s\n", furi->toString().c_str(),
                    struc->pattern()->toString().c_str(), source->toString().c_str());
      return struc->read(furi, source);
    }

    [[nodiscard]] RESPONSE_CODE write(const ID_p &id, const Obj_p &obj,
                                      const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID),
                                      const bool retain = RETAIN_MESSAGE) const {
      const Structure_p &struc = this->get_structure(p_p(*id), source);
      LOG_STRUCTURE(DEBUG, this, "!y!_writing!! %s to !b%s!! from " FURI_WRAP " for %s\n", obj->toString().c_str(),
                    id->toString().c_str(), struc->pattern()->toString().c_str(), source->toString().c_str());
      struc->write(id, obj, source, retain);
      return OK;
    }

    void remove(const ID_p &id, const ID_p &source = id_p(FOS_DEFAULT_SOURCE_ID)) const {
      const Structure_p &struc = this->get_structure(p_p(*id), source);
      LOG_STRUCTURE(DEBUG, this, "!y!_removing!! !b%s!! from " FURI_WRAP " for %s\n", id->toString().c_str(),
                    struc->pattern()->toString().c_str(), source->toString().c_str());
      struc->remove(id, source);
    }

    [[nodiscard]] RESPONSE_CODE route_message(const Message_p &message) const {
      const Structure_p &struc = this->get_structure(p_p(message->target), id_p(message->source));
      LOG_STRUCTURE(DEBUG, this, "!y!_routing message!! %s\n", message->toString().c_str());
      return struc->recv_message(message);
    }

    RESPONSE_CODE route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) const {
      for (const Structure_p &struc: *this->structures_ | views::values) {
        if (struc->pattern()->matches(*pattern)) {
          LOG_STRUCTURE(DEBUG, this, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                        subscriber->toString().c_str());
          struc->recv_unsubscribe(subscriber, pattern);
        }
      }
      return OK;
    }

    RESPONSE_CODE route_subscription(const Subscription_p &subscription) const {
      const Structure_p &struc = this->get_structure(p_p(subscription->pattern), id_p(subscription->source));
      LOG_STRUCTURE(DEBUG, this, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
      return OK;
    }

  private:
    [[nodiscard]] Structure_p &get_structure(const Pattern_p &pattern, const ID_p &source) const {
      Structure_p *ret = nullptr;
      for (Structure_p &struc: *this->structures_ | views::values) {
        if (pattern->matches(*struc->pattern())) {
          if (ret != nullptr)
            throw fError(FURI_WRAP " too general as it crosses multiple structures\n ", pattern->toString().c_str());
          ret = &struc;
        }
      }
      if (nullptr == ret)
        throw fError(FURI_WRAP " has no structure to contain !b%s!! for !b%s!!\n", this->pattern()->toString().c_str(),
                     pattern->toString().c_str(), source->toString().c_str());
      return *ret;
    }
  };

  inline ptr<Router> router() {
    return Options::singleton()->router<Router>();
  }
} // namespace fhatos

#endif
