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
#include <util/obj_helper.hpp>

namespace fhatos {
  class Router final : public Patterned {
  protected:
    MutexRW<> structures_mutex_ = MutexRW<>("<router structures mutex>");
    List<Structure_p> structures_ = List<Structure_p>();

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/") {
      static auto router_p = ptr<Router>(new Router(pattern));
      static bool setup = false;
      if (!setup)
        setup = true;
      return router_p;
    }

    void loop() {
      for (const Structure_p &structure: this->structures_) {
        structure->loop();
      }
    }

    void stop() {
      auto *ephemeral_count = new atomic_int(0);
      auto *local_count = new atomic_int(0);
      auto *network_count = new atomic_int(0);
      this->structures_mutex_.read<void *>([this, ephemeral_count, local_count, network_count]() {
        for (const Structure_p &structure: this->structures_) {
          switch (structure->stype) {
            case SType::EPHEMERAL:
              ephemeral_count->fetch_add(1);
              break;
            case SType::LOCAL:
              local_count->fetch_add(1);
              break;
            case SType::NETWORK:
              network_count->fetch_add(1);
              break;
          }
        }
        return nullptr;
      });
      LOG_ROUTER(INFO, "!yStopping!g %i !yephemeral!! | !g%i !yram!! | !g%i !ynetwork!!\n", ephemeral_count->load(),
                 local_count->load(), network_count->load());
      delete ephemeral_count;
      delete local_count;
      delete network_count;
      this->detach(p_p("#"));
      LOG_ROUTER(INFO, "!yrouter !b%s!! stopped\n", this->pattern()->toString().c_str());
    }

    void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_ROUTER(INFO, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                   StructureTypes.to_chars(structure->stype).c_str());
      } else {
        this->structures_mutex_.write<void *>([this, structure]() {
          for (const Structure_p &s: this->structures_) {
            if (structure->pattern()->bimatches(*s->pattern())) {
              // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
              throw fError(ROUTER_FURI_WRAP
                           " Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                           this->pattern_->toString().c_str(), s->pattern()->toString().c_str(),
                           structure->pattern()->toString().c_str());
            }
          }

          this->structures_.push_back(structure);
          structure->setup();
          if (structure->available()) {
            LOG_ROUTER(INFO, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                       StructureTypes.to_chars(structure->stype).c_str());
          } else {
            LOG_ROUTER(ERROR, "!RUnable to attach %s: %s!!\n", StructureTypes.to_chars(structure->stype).c_str(),
                       structure->pattern()->toString().c_str());
            this->structures_.pop_back();
          }
          return nullptr;
        });
      }
    }

    void detach(const Pattern_p &pattern) {
      this->structures_mutex_.write<void *>([this, pattern]() {
        this->structures_.erase(remove_if(this->structures_.begin(), this->structures_.end(),
                                          [this, pattern](const Structure_p &structure) -> bool {
                                            const bool to_erase = structure->pattern()->matches(*pattern);
                                            if (to_erase) {
                                              LOG_ROUTER(INFO, FURI_WRAP " !y%s!! detached\n",
                                                         structure->pattern()->toString().c_str(),
                                                         StructureTypes.to_chars(structure->stype).c_str());
                                            }
                                            return to_erase;
                                          }),
                                this->structures_.end());
        return nullptr;
      });
    }

    [[nodiscard]] Objs_p read(const fURI_p &furi) const {
      ////////////////////////////////////////////////////////
      ///////////////////// ROUTER READS /////////////////////
      ////////////////////////////////////////////////////////
      if (this->pattern()->resolve("./structure/").bimatches(*furi)) {
        List<Uri_p> uris;
        for (const Structure_p &structure: this->structures_) {
          uris.push_back(vri(structure->pattern()));
        }
        const Rec_p rec = ObjHelper::encode_lst(this->pattern()->resolve("./structure/"), uris);
        return rec;
      }
      if (this->pattern()->resolve("./structure/+").bimatches(*furi)) {
        if (StringHelper::is_integer(furi->name()))
          return vri(this->structures_.at(stoi(furi->name()))->pattern());
        if (furi->name() == "+" || furi->name() == "#") {
          const Objs_p objs = Obj::to_objs();
          for (const Structure_p &structure: this->structures_) {
            objs->add_obj(vri(structure->pattern()));
          }
          return objs;
        }
      }
      //////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////
      const Structure_p &struc = this->get_structure(p_p(*furi));
      LOG_ROUTER(DEBUG, "!y!_reading!! !b%s!! from " FURI_WRAP "\n", furi->toString().c_str(),
                 struc->pattern()->toString().c_str());
      return struc->read(furi);
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN_MESSAGE) {
      if (!ROUTER_INTERCEPT(*furi, obj, retain)) {
        const Structure_p &structure = this->get_structure(p_p(*furi));
        LOG_ROUTER(DEBUG, "!y!_writing!! !g%s!! %s to !b%s!! at " FURI_WRAP "\n", retain ? "retained" : "transient",
                   obj->toString().c_str(), furi->toString().c_str(), structure->pattern()->toString().c_str());
        structure->write(furi, obj, retain);
        SCHEDULER_INTERCEPT(*furi, obj, retain);
      }
    }

    void remove(const ID_p &id) const {
      const Structure_p &struc = this->get_structure(p_p(*id));
      LOG_ROUTER(DEBUG, "!y!_removing!! !b%s!! from " FURI_WRAP "\n", id->toString().c_str(),
                 struc->pattern()->toString().c_str());
      struc->remove(id);
    }

    /*void route_message(const Message_p &message) const {
      const Structure_p &struc = this->get_structure(p_p(message->target));
      LOG_ROUTER(DEBUG, "!y!_routing message!! %s\n", message->toString().c_str());
      struc->recv_publication(message);
    }*/

    void route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) const {
      for (const Structure_p &structure: this->structures_) {
        if (structure->pattern()->matches(*pattern) || pattern->matches(*structure->pattern())) {
          LOG_ROUTER(DEBUG, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                     subscriber->toString().c_str());
          structure->recv_unsubscribe(subscriber, pattern);
        }
      }
    }

    void route_subscription(const Subscription_p &subscription) const {
      const Structure_p &struc = this->get_structure(p_p(subscription->pattern));
      LOG_ROUTER(DEBUG, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    }

  private:
    [[nodiscard]] const Structure_p &get_structure(const Pattern_p &pattern) const {
      //  return *this->structures_mutex_.read<Structure_p *>([this,pattern]() {
      const Structure_p *ret = nullptr;
      const Pattern_p temp = pattern->is_branch() ? p_p(pattern->extend("+")) : pattern;
      for (const Structure_p &structure: this->structures_) {
        if (temp->matches(*structure->pattern())) {
          /// TODO: should be flipped?
          if (ret != nullptr)
            throw fError(ROUTER_FURI_WRAP " too general as it crosses multiple structures",
                         pattern->toString().c_str());
          ret = &structure;
        }
      }
      if (nullptr == ret)
        throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->pattern()->toString().c_str(),
                     pattern->toString().c_str());
      return *ret;
      //});
    }

  protected:
    explicit Router(const Pattern &pattern) : Patterned(p_p(pattern)) {
      ROUTER_INTERCEPT = [this](const fURI &furi, const Obj_p &payload, const bool retain) -> bool {
        if (!retain)
          return false;
        if (payload->is_noobj()) {
          Structure_p found = nullptr;
          for (const auto &structure: this->structures_) {
            if (structure->pattern()->equals(furi)) {
              found = structure;
            }
          }
          if (!found)
            return false;
          found->stop();
          this->detach(found->pattern());
          return true;
        }
        if (payload->is_rec() &&
            (payload->id()->matches(LOCAL_FURI->extend("#")) || payload->id()->matches(NETWORK_FURI->extend("#")) ||
             payload->id()->matches(EXTERNAL_FURI->extend("#")))) {
          LOG_ROUTER(DEBUG, "intercepting retained %s\n", payload->toString().c_str());
          STRUCTURE_ATTACHER(furi, payload);
          return true;
        }
        return false;
      };
      LOG_ROUTER(INFO, "!yrouter!! started\n");
    }
  };

  inline ptr<Router> router() { return Options::singleton()->router<Router>(); }
} // namespace fhatos

#endif
