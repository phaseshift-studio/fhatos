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

#include <fhatos.hpp>
#include <structure/structure.hpp>
#include <util/mutex_rw.hpp>
#include <util/obj_helper.hpp>


namespace fhatos {
  class Router final : public Patterned {
  protected:
    MutexDeque<Structure_p> structures_ = MutexDeque<Structure_p>();

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/") {
      static auto router_p = ptr<Router>(new Router(pattern));
      static bool setup = false;
      if (!setup)
        setup = true;
      return router_p;
    }

    void loop() {
      this->structures_.forEach([](const Structure_p &structure) {
        structure->loop();
      });
    }

    void stop() {
      auto *ephemeral_count = new atomic_int(0);
      auto *local_count = new atomic_int(0);
      auto *network_count = new atomic_int(0);
      this->structures_.forEach([ephemeral_count,local_count,network_count](const Structure_p &structure) {
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
        this->structures_.forEach([structure,this](const Structure_p &s) {
          if (structure->pattern()->bimatches(*s->pattern())) {
            // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
            throw fError(ROUTER_FURI_WRAP
                         " Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                         this->pattern_->toString().c_str(), s->pattern()->toString().c_str(),
                         structure->pattern()->toString().c_str());
          }
        });
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
      }
    }

    void detach(const Pattern_p &pattern) {
      this->structures_.remove_if([this, pattern](const Structure_p &structure) -> bool {
        const bool to_erase = structure->pattern()->matches(*pattern);
        if (to_erase) {
          LOG_ROUTER(INFO, FURI_WRAP " !y%s!! detached\n",
                     structure->pattern()->toString().c_str(),
                     StructureTypes.to_chars(structure->stype).c_str());
        }
        return to_erase;
      });
    }

    [[nodiscard]] Objs_p read(const fURI_p &furi) {
      ////////////////////////////////////////////////////////
      //////////// ROUTER/SCEDULER READ INTERCEPTS ///////////
      ////////////////////////////////////////////////////////
      const Objs_p objs = Obj::to_objs();
      objs->add_obj(ROUTER_READ_INTERCEPT(*furi));
      objs->add_obj(SCHEDULER_READ_INTERCEPT(*furi));
      if (!objs->objs_value()->empty())
        return objs;
      //////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////
      const Structure_p &struc = this->get_structure(*furi);
      const Obj_p obj = struc->read(furi);
      LOG_ROUTER(DEBUG, "!g!_reading!! !g[!b%s!m=>!y%s!g]!! from " FURI_WRAP "\n",
                 furi->toString().c_str(),
                 obj->toString().c_str(),
                 struc->pattern()->toString().c_str());
      return obj;
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN_MESSAGE) {
      if (!ROUTER_WRITE_INTERCEPT(*furi, obj, retain)) {
        const Structure_p &structure = this->get_structure(*furi);
        LOG_ROUTER(DEBUG, "!g!_writing!! %s !g[!b%s!m=>!y%s!g]!! to " FURI_WRAP "\n", retain ? "retained" : "transient",
                   furi->toString().c_str(), obj->toString().c_str(), structure->pattern()->toString().c_str());
        structure->write(furi, obj, retain);
        SCHEDULER_WRITE_INTERCEPT(*furi, obj, retain);
      }
    }

    void route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) {
      this->structures_.forEach([this,subscriber,pattern](const Structure_p &structure) {
        if (structure->pattern()->matches(*pattern) || pattern->matches(*structure->pattern())) {
          LOG_ROUTER(DEBUG, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                     subscriber->toString().c_str());
          structure->recv_unsubscribe(subscriber, pattern);
        }
      });
    }

    void route_subscription(const Subscription_p &subscription) {
      const Structure_p &struc = this->get_structure(subscription->pattern);
      LOG_ROUTER(DEBUG, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    }

  private:
    [[nodiscard]] Structure_p get_structure(const Pattern &pattern) {
      const Pattern temp = pattern.is_branch() ? Pattern(pattern.extend("+")) : pattern;
      const List<Structure_p> list = this->structures_.find_all([this,pattern,temp](const Structure_p &structure) {
        return pattern.matches(*structure->pattern()) || temp.matches(*structure->pattern());
      }, false); // TODO: NO MUTEX!
      if (list.size() > 1)
        throw fError(ROUTER_FURI_WRAP " too general as it crosses multiple structures",
                     pattern.toString().c_str());
      if (list.empty())
        throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->pattern()->toString().c_str(),
                     pattern.toString().c_str());
      const Structure_p s = list.front();
      return s;
    }

  protected:
    explicit Router(const Pattern &pattern) : Patterned(p_p(pattern)) {
      ROUTER_WRITE_AT = [this](const ID_p &id, const Obj_p &obj) -> const Obj_p {
        this->write(id, obj, true);
        return obj;
      };
      ROUTER_WRITE_INTERCEPT = [this](const fURI &furi, const Obj_p &payload, const bool retain) -> bool {
        if (!retain)
          return false;
        if (payload->is_noobj()) {
          const Option<Structure_p> found = this->structures_.find([furi](const Structure_p &structure) {
            return structure->pattern()->equals(furi);
          });
          if (!found.has_value())
            return false;
          found.value()->stop();
          this->detach(found.value()->pattern());
          return true;
        }
        if (payload->is_rec() &&
            (payload->type()->matches(LOCAL_FURI->extend("#")) || payload->type()->matches(NETWORK_FURI->extend("#")) ||
             payload->type()->matches(EXTERNAL_FURI->extend("#")))) {
          LOG_ROUTER(DEBUG, "intercepting retained %s\n", payload->toString().c_str());
          STRUCTURE_ATTACHER(furi, payload);
          return true;
        }
        return false;
      };
      ROUTER_READ_INTERCEPT = [this](const fURI &furi) -> Objs_p {
        if (this->pattern()->resolve("./structure/").bimatches(furi)) {
          auto uris = make_shared<List<Uri_p>>();
          this->structures_.forEach([uris](const Structure_p &structure) {
            uris->push_back(vri(structure->pattern()));
          });
          const Rec_p rec = ObjHelper::encode_lst(this->pattern()->resolve("./structure/"), *uris);
          return rec;
        }
        if (this->pattern()->resolve("./structure/+").bimatches(furi)) {
          if (StringHelper::is_integer(furi.name())) {
            const Option<Structure_p> option = this->structures_.get(stoi(furi.name()));
            if (!option.has_value())
              throw fError("no structure at provided index %i", stoi(furi.name()));
            return vri(option.value()->pattern());
          }
          if (furi.name() == "+" || furi.name() == "#") {
            const Objs_p objs = Obj::to_objs();
            this->structures_.forEach([objs](const Structure_p &structure) {
              objs->add_obj(vri(structure->pattern()));
            });
            return objs;
          }
        }
        return noobj();
      };
      LOG_ROUTER(INFO, "!yrouter!! started\n");
    }
  };

  inline ptr<Router> router() { return Options::singleton()->router<Router>(); }
} // namespace fhatos

#endif
