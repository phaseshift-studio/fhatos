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
#define FOS_NAMESPACE_PREFIX_ID MMADT_SCHEME "/uri/ns/prefix/"
#endif

#include "../fhatos.hpp"
#include "../structure/structure.hpp"
#include FOS_MQTT(mqtt.hpp)
#include "../structure/stype/heap.hpp"

namespace fhatos {
  /*static IdObjPairs_p make_id_objs(initializer_list<Pair<ID_p, Obj_p>> init = {}) {
    return make_shared<IdObjPairs>(init);
  }*/

  class Router final : public Rec {
  protected:
    const ID_p namespace_prefix_;
    // List<fURI> prefixes = List<fURI>();
    MutexDeque<Structure_p> structures_ = MutexDeque<Structure_p>();

  protected:
    explicit Router(const ID &id, const ID &namespace_prefix = FOS_NAMESPACE_PREFIX_ID) : Rec(rmap({
          {"structure", to_lst()},
          {"resolve", to_rec({
            {"namespace", to_rec({{":", vri("/mmadt/")}, {"fos:", vri("/fos/")}, {"math:", vri("/mmadt/ext/math")}})},
            {"auto_prefix", to_lst({vri(""), vri("/mmadt/"), vri("/fos/"), vri("/sys/")})}})},
          {":stop", to_inst(
            [this](const Obj_p &, const InstArgs &) {
              this->write(this->vid_, _noobj_);
              this->stop();
              return noobj();
            }, NO_ARGS, INST_FURI,
            make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__, __LINE__)))},
          {":attach", to_inst(
            [this](const Obj_p &obj, const InstArgs &args) {
              if(args.at(0)->tid()->name() == "heap")
                this->attach(make_shared<Heap<>>(obj));
              else if(args.at(0)->tid()->name() == "mqtt")
                this->attach(make_shared<Mqtt>(obj));
              return noobj();
            }, {x(0, ___)}, INST_FURI,
            make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__, __LINE__)))}}),
        OType::REC, REC_FURI, id_p(id)),
      namespace_prefix_(id_p(namespace_prefix)) {
      ROUTER_ID = this->vid_;
      ////////////////////////////////////////////////////////////////////////////////////
      ROUTER_RESOLVE = [this](const fURI &furi) -> fURI_p {
        if(!furi.headless() && !furi.has_components())
          return furi_p(furi);
        List<fURI> components = furi.has_components() ? List<fURI>() : List<fURI>{furi};
        if(furi.has_components()) {
          for(const auto &c: furi.components()) {
            components.emplace_back(fURI(c));
          }
        }
        bool first = true;
        fURI_p test = nullptr;
        for(const auto &c: components) {
          List_p<Uri_p> prefixes = this->rec_get("resolve")->rec_get("auto_prefix")->lst_value();
          // TODO: make this an exposed property of /sys/router
          fURI_p found = nullptr;
          for(const auto &prefix: *prefixes) {
            const fURI_p x = furi_p(prefix->uri_value().extend(c));
            if(const Structure_p structure = this->get_structure(*x, false); structure && structure->has(x)) {
              LOG_KERNEL_OBJ(TRACE, this, "located !b%s!! in %s and resolved to !b%s!!\n",
                             furi.toString().c_str(),
                             structure->toString().c_str(),
                             x->toString().c_str());
              found = x;
              break;
            }
          }
          if(!found) {
            LOG_KERNEL_OBJ(TRACE, this, "unable to locate !b%s!!\n", c.toString().c_str());
          }
          if(first) {
            first = false;
            test = furi_p(fURI(found ? *found : c));
          } else {
            test = furi_p(test->extend("::").extend(found ? *found : fURI(c)));
          }
        }
        return test;
      };
      ROUTER_READ = [this](const fURI_p &furix) -> Obj_p { return this->read(furix); };
      ROUTER_WRITE = [this](const fURI_p &furix, const Obj_p &obj, const bool retain) -> const Obj_p {
        this->write(furix, obj, retain);
        return obj;
      };
      ROUTER_SUBSCRIBE = [this](const Subscription_p &subscription) { this->route_subscription(subscription); };
      LOG_KERNEL_OBJ(INFO, this, "!yrouter!! started\n");
    }

  public:
    static ptr<Router> singleton(const Pattern &pattern = "/sys/router/",
                                 const ID &namespace_prefix = ID(FOS_NAMESPACE_PREFIX_ID)) {
      static auto router_p = ptr<Router>(new Router(pattern, namespace_prefix));
      return router_p;
    }

    void loop() {
      if(!this->structures_
        .remove_if([this](const Structure_p &structure) {
          const bool online = structure->available();
          if(online)
            structure->loop();
          else {
            LOG_KERNEL_OBJ(INFO, this, FURI_WRAP " !y%s!! detached\n", structure->pattern()->toString().c_str(),
                           structure->tid()->name().c_str());
          }
          return !online;
        })
        ->empty())
        this->save();
    }

    void stop() {
      auto map = make_shared<Map<string, int>>();
      this->structures_.forEach([map](const Structure_p &structure) {
        const string name = structure->tid()->name();
        int count = map->count(name) ? map->at(name) : 0;
        count++;
        if(map->count(name))
          map->erase(name);
        map->insert({name, count});
      });
      for(const auto &[name, count]: *map) {
        LOG_KERNEL_OBJ(INFO, this, "!b%i !y%s!!(s) closing\n", count, name.c_str());
      }
      this->structures_.forEach([](const Structure_p &structure) { structure->stop(); });
      LOG_KERNEL_OBJ(INFO, this, "!yrouter !b%s!! stopped\n", this->vid()->toString().c_str());
    }

    void attach(const Structure_p &structure) {
      if(structure->pattern()->equals(Pattern(""))) {
        LOG_KERNEL_OBJ(INFO, this, "!b%s!! !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                       structure->tid()->name().c_str());
      } else {
        this->structures_.forEach([structure, this](const Structure_p &s) {
          if(structure->pattern()->bimatches(*s->pattern())) {
            // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
            throw fError(ROUTER_FURI_WRAP
                         " Only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                         this->vid_->toString().c_str(), s->pattern()->toString().c_str(),
                         structure->pattern()->toString().c_str());
          }
        });
        this->structures_.push_back(structure);
        structure->setup();
        if(structure->available()) {
          LOG_KERNEL_OBJ(INFO, this, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                         structure->tid()->name().c_str());
        } else {
          LOG_KERNEL_OBJ(ERROR, this, "!runable to attach %s: %s!!\n", structure->pattern()->toString().c_str(),
                         structure->tid()->name().c_str());
          this->structures_.pop_back();
        }
      }
      this->save();
    }


    virtual void save(const ID_p & = nullptr) override {
      const Lst_p strcs = Obj::to_lst();
      this->structures_.forEach([strcs](const Structure_p &structure) { strcs->lst_add(vri(structure->pattern())); });
      this->rec_set("structure", strcs);
      Obj::save();
    }

    [[nodiscard]] Obj_p exec(const ID_p &bcode_id, const Obj_p &arg) { return this->read(bcode_id)->apply(arg); }

    [[nodiscard]] Objs_p read(const fURI_p &furi) {
      try {
        const fURI_p resolved_furi = ROUTER_RESOLVE(*furi);
        const bool query = resolved_furi->has_query("structure");
        const Structure_p structure = this->get_structure(*resolved_furi);
        if(query)
          return structure->shared_from_this();
        const Objs_p objs = structure->read(resolved_furi);
        LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_reading!! !g[!b%s!m=>!y%s!g]!! from " FURI_WRAP "\n",
                       Process::current_process()->vid()->toString().c_str(), resolved_furi->toString().c_str(),
                       objs->toString().c_str(), structure->pattern()->toString().c_str());
        return objs->none_one_all();
      } catch(const fError &e) {
        LOG_EXCEPTION(this->shared_from_this(), e);
        return noobj();
      }
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN) {
      try {
        const Structure_p structure = this->get_structure(*furi);
        LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_writing!! %s !g[!b%s!m=>!y%s!g]!! to " FURI_WRAP "\n",
                       Process::current_process()->vid()->toString().c_str(), retain ? "retained" : "transient",
                       furi->toString().c_str(), obj->tid()->toString().c_str(),
                       structure->pattern()->toString().c_str());
        structure->write(furi, obj, retain);
      } catch(const fError &e) {
        LOG_EXCEPTION(this->shared_from_this(), e);
      }
    }

    void route_unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#")) {
      try {
        this->structures_.forEach([this, subscriber, pattern](const Structure_p &structure) {
          if(structure->pattern()->matches(*pattern) || pattern->matches(*structure->pattern())) {
            LOG_KERNEL_OBJ(DEBUG, this, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern->toString().c_str(),
                           subscriber->toString().c_str());
            structure->recv_unsubscribe(subscriber, pattern);
          }
        });
      } catch(const fError &e) {
        LOG_EXCEPTION(this->shared_from_this(), e);
      }
    }

    void route_subscription(const Subscription_p &subscription) {
      try {
        const Structure_p struc = this->get_structure(subscription->pattern());
        LOG_KERNEL_OBJ(DEBUG, this, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
        struc->recv_subscription(subscription);
      } catch(const fError &e) {
        LOG_EXCEPTION(this->shared_from_this(), e);
      }
    }

    static void *import() {
      ROUTER_WRITE(ROUTER_ID, Router::singleton(),RETAIN);
      ROUTER_WRITE(id_p(ROUTER_ID->extend("lib/heap")), make_shared<Heap<>>(Obj::to_rec({{"pattern", vri("#")}})),
                   RETAIN);
      ROUTER_WRITE(id_p("/sys/router/lib/mqtt"),
                   make_shared<Mqtt>(Obj::to_rec({
                     {"pattern", vri("#")},
                     {"broker", vri("#")},
                     {"client", vri("#")}})),
                   RETAIN);
      return nullptr;
    }

  private:
    [[nodiscard]] Structure_p get_structure(const Pattern &pattern, const bool throw_exception = true) {
      const Pattern temp = pattern.is_branch() ? Pattern(pattern.extend("+")) : pattern;
      const List<Structure_p> list = this->structures_.find_all(
        [pattern, temp](const Structure_p &structure) {
          return pattern.matches(*structure->pattern()) || temp.matches(*structure->pattern());
        },
        false); // TODO: NO MUTEX!
      if(throw_exception) {
        if(list.size() > 1)
          throw fError(ROUTER_FURI_WRAP " crosses multiple structures", pattern.toString().c_str());
        if(list.empty())
          throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->vid()->toString().c_str(),
                       pattern.toString().c_str());
        const Structure_p s = list.at(0);
        return s;
      }
      const Structure_p s = list.size() == 1 ? list.at(0) : nullptr;
      return s;
    }

    [[nodiscard]] fURI_p resolve_namespace_prefix(const fURI_p &type_id) {
      fURI_p type_id_resolved;
      if(strlen(type_id->scheme()) > 0) {
        const Obj_p resolved_uri = this->read(id_p(namespace_prefix_->extend(type_id->scheme())));
        LOG_KERNEL_OBJ(DEBUG, this, "!g!_resolving !y%s!!:!b%s!! to !b%s!!\n", type_id->scheme(),
                       type_id->path().c_str(),
                       resolved_uri->toString().c_str());
        if(!resolved_uri->is_noobj()) {
          if(resolved_uri->is_uri()) {
            return id_p(resolved_uri->uri_value().extend((string(":").append(type_id->path())).c_str()));
          } else
            throw fError("namespace prefixes must be uris: !y%s!!:!b%s!!", resolved_uri->toString().c_str());
        }
      }
      return type_id; // no resolution available
    }
  };

  inline ptr<Router> router() { return Router::singleton(); }
} // namespace fhatos

#endif
