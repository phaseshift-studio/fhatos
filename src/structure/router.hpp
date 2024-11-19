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

#ifndef FOS_NAMESPACE_PREFIX_ID
#define FOS_NAMESPACE_PREFIX_ID FOS_TYPE_PREFIX "uri/ns/prefix/"
#endif

#include <fhatos.hpp>
#include <structure/structure.hpp>
#include <util/obj_helper.hpp>

namespace fhatos {
  /*static IdObjPairs_p make_id_objs(initializer_list<Pair<ID_p, Obj_p>> init = {}) {
    return make_shared<IdObjPairs>(init);
  }*/

  class Router final : public Rec {
  protected:
    const ID_p namespace_prefix_;
    MutexDeque<Structure_p> structures_ = MutexDeque<Structure_p>();

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/",
                                 const ID &namespace_prefix = ID(FOS_NAMESPACE_PREFIX_ID)) {
      static auto router_p = ptr<Router>(new Router(pattern, namespace_prefix));
      return router_p;
    }

    void loop() {
      this->structures_.forEach([](const Structure_p &structure) { structure->loop(); });
    }

    void stop() {
      auto *computed_count = new atomic_int(0);
      auto *heap_count = new atomic_int(0);
      auto *mqtt_count = new atomic_int(0);
      this->structures_.forEach([computed_count, heap_count, mqtt_count](const Structure_p &structure) {
        switch (structure->stype) {
          case SType::COMPUTED:
            computed_count->fetch_add(1);
            break;
          case SType::HEAP:
            heap_count->fetch_add(1);
            break;
          case SType::MQTT:
            mqtt_count->fetch_add(1);
            break;
        }
      });
      LOG_ROUTER(INFO, "!yStopping!g %i !ycomputed!! | !g%i !yheap!! | !g%i !ymqtt!!\n", computed_count->load(),
                 heap_count->load(), mqtt_count->load());
      delete computed_count;
      delete heap_count;
      delete mqtt_count;
      this->detach(p_p("#"));
      LOG_ROUTER(INFO, "!yrouter !b%s!! stopped\n", this->vid()->toString().c_str());
    }

    void attach(const ptr<Structure> &structure) {
      if (structure->pattern()->equals(Pattern(""))) {
        LOG_ROUTER(INFO, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                   StructureTypes.to_chars(structure->stype).c_str());
      } else {
        this->structures_.forEach([structure, this](const Structure_p &s) {
          if (structure->pattern()->bimatches(*s->pattern())) {
            // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
            throw fError(ROUTER_FURI_WRAP
                         " Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                         this->vid_->toString().c_str(), s->pattern()->toString().c_str(),
                         structure->pattern()->toString().c_str());
          }
        });
        this->structures_.push_back(structure);
        structure->setup();
        if (structure->available()) {
          LOG_ROUTER(INFO, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                     StructureTypes.to_chars(structure->stype).c_str());
        } else {
          LOG_ROUTER(ERROR, "!runable to attach %s: %s!!\n", StructureTypes.to_chars(structure->stype).c_str(),
                     structure->pattern()->toString().c_str());
          this->structures_.pop_back();
        }
      }
      this->save();
    }

    void detach(const Pattern_p &pattern) {
      this->structures_.remove_if([this, pattern](const Structure_p &structure) -> bool {
        const bool to_erase = structure->pattern()->matches(*pattern);
        if (to_erase) {
          structure->stop();
          LOG_ROUTER(INFO, FURI_WRAP " !y%s!! detached\n", structure->pattern()->toString().c_str(),
                     StructureTypes.to_chars(structure->stype).c_str());
        }
        return to_erase;
      });
    }

    virtual Obj_p save(const ID_p & = nullptr) override {
      const Lst_p strcs = Obj::to_lst();
      this->structures_.forEach([strcs](const Structure_p &proc) {
        strcs->lst_add(vri(proc->pattern()));
      });
      this->rec_set("structure", strcs);
      this->write(this->vid(), shared_from_this());
      return shared_from_this();
    }

    [[nodiscard]] Obj_p exec(const ID_p &bcode_id, const Obj_p &arg) { return this->read(bcode_id)->apply(arg); }

    [[nodiscard]] Objs_p read(const fURI_p &furi) {
      const fURI_p resolved_furi = resolve_namespace_prefix(furi);
      const Structure_p &struc = this->get_structure(*resolved_furi);
      const Objs_p objs = struc->read(resolved_furi);
      LOG_ROUTER(DEBUG, FURI_WRAP " !g!_reading!! !g[!b%s!m=>!y%s!g]!! from " FURI_WRAP "\n",
                 Process::current_process()->vid()->toString().c_str(), resolved_furi->toString().c_str(),
                 objs->toString().c_str(), struc->pattern()->toString().c_str());
      return objs->none_one_all();
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN) {
      const Structure_p &structure = this->get_structure(*furi);
      LOG_ROUTER(DEBUG, FURI_WRAP " !g!_writing!! %s !g[!b%s!m=>!y%s!g]!! to " FURI_WRAP "\n",
                 Process::current_process()->vid()->toString().c_str(), retain ? "retained" : "transient",
                 furi->toString().c_str(), obj->tid()->toString().c_str(), structure->pattern()->toString().c_str());
      structure->write(furi, obj, retain);
    }

    void route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) {
      this->structures_.forEach([this, subscriber, pattern](const Structure_p &structure) {
        if (structure->pattern()->matches(*pattern) || pattern->matches(*structure->pattern())) {
          LOG_ROUTER(DEBUG, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                     subscriber->toString().c_str());
          structure->recv_unsubscribe(subscriber, pattern);
        }
      });
    }

    void route_subscription(const Subscription_p &subscription) {
      const Structure_p &struc = this->get_structure(subscription->pattern());
      LOG_ROUTER(DEBUG, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    }

  private:
    [[nodiscard]] Structure_p get_structure(const Pattern &pattern) {
      const Pattern temp = pattern.is_branch() ? Pattern(pattern.extend("+")) : pattern;
      const List<Structure_p> list = this->structures_.find_all(
          [pattern, temp](const Structure_p &structure) {
            return pattern.matches(*structure->pattern()) || temp.matches(*structure->pattern());
          },
          false); // TODO: NO MUTEX!
      if (list.size() > 1)
        throw fError(ROUTER_FURI_WRAP " too general as it crosses multiple structures", pattern.toString().c_str());
      if (list.empty())
        throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->vid()->toString().c_str(),
                     pattern.toString().c_str());
      const Structure_p s = list.front();
      return s;
    }

    [[nodiscard]] fURI_p resolve_namespace_prefix(const fURI_p &type_id) {
      fURI_p type_id_resolved;
      if (strlen(type_id->scheme()) > 0) {
        const Obj_p resolved_uri = this->read(id_p(namespace_prefix_->extend(type_id->scheme())));
        LOG_ROUTER(DEBUG, "!g!_resolving !y%s!!:!b%s!! to !b%s!!\n",
                   type_id->scheme(),
                   type_id->path().c_str(),
                   resolved_uri->toString().c_str());
        if (!resolved_uri->is_noobj()) {
          if (resolved_uri->is_uri()) {
            return id_p(resolved_uri->uri_value().extend((string(":").append(type_id->path())).c_str()));
          } else
            throw fError("namespace prefixes must be uris: !y%s!!:!b%s!!", resolved_uri->toString().c_str());
        }
      }
      return type_id; // no resolution available
    }

  protected:
    explicit Router(const ID &id, const ID &namespace_prefix = FOS_NAMESPACE_PREFIX_ID) :
      Rec(rmap({
              {"structure", to_lst()},
              {":stop", to_bcode([this](const Obj_p &) {
                this->stop();
                return noobj();
              }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
              {":attach", to_bcode([this](const Obj_p &) {
                //this->attach();
                return noobj();
              }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))}
          }), OType::REC, REC_FURI, id_p(id)),
      namespace_prefix_(id_p(namespace_prefix)) {
      ROUTER_READ = [this](const fURI_p &furix) -> Obj_p { return this->read(furix); };
      ROUTER_WRITE = [this](const ID_p &idx, const Obj_p &obj, const bool retain) -> const Obj_p {
        this->write(idx, obj, retain);
        return obj;
      };
      ROUTER_SUBSCRIBE = [this](const Subscription_p &subscription) {
        this->route_subscription(subscription);
      };
      LOG_ROUTER(INFO, "!yrouter!! started\n");
    }
  };

  inline ptr<Router> router() { return Router::singleton(); }
} // namespace fhatos

#endif