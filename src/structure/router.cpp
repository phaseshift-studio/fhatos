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

#include "router.hpp"
#include "../util/obj_helper.hpp"
#include "../structure/stype/frame.hpp"

namespace fhatos {
  inline thread_local ptr<Frame<>> THREAD_FRAME_STACK = nullptr;

  ptr<Router> Router::singleton(const ID &value_id) {
    static auto router_p = std::make_shared<Router>(value_id);
    return router_p;
  }

  Router::Router(const ID &id) : Rec(rmap({
                                       {"structure", to_lst()},
                                       {"resolve", to_rec({
                                         {"namespace",
                                           to_rec({{":", vri("/mmadt/")}, {"fos:", vri("/fos/")},
                                             {"math:", vri("/mmadt/ext/math")}})},
                                         {"auto_prefix",
                                           to_lst({vri(""), vri("/mmadt/"), vri("/fos/"), vri("/sys/")})}})}}),
                                     /*{":stop", to_inst(
                                       [this](const Obj_p &, const InstArgs &) {
                                         this->write(this->vid_, _noobj_);
                                         this->stop();
                                         return noobj();
                                       }, %s, INST_FURI,
                                       make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__, __LINE__)))},
                                     {":attach", to_inst(
                                       [this](const Obj_p &obj, const InstArgs &args) {
                                         if(args->arg(0)->tid_->name() == "heap")
                                           this->attach(make_shared<Heap<>>(obj));
                                         else if(args->arg(0)->tid_->name() == "mqtt")
                                           this->attach(make_shared<Mqtt>(obj));
                                         return noobj();
                                       }, {x(0, Obj::to_bcode())}, INST_FURI,
                                       make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__, __LINE__)))}}*/
                                     OType::REC, REC_FURI, id_p(id)),
                                 structures_(make_unique<MutexDeque<Structure_p>>()) {
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_ID = id_p(this->vid_);
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_PUSH_FRAME = [this](const Pattern &pattern, const Rec_p &frame_data) {
      this->push_frame(pattern, frame_data);
    };
    ROUTER_POP_FRAME = [this] {
      this->pop_frame();
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_RESOLVE = [this](const fURI &furi) -> fURI_p {
      return this->resolve(furi);
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_READ = [this](const fURI_p &furi) -> Obj_p {
      return this->read(furi);
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_WRITE = [this](const fURI_p &furi, const Obj_p &obj, const bool retain) {
      this->write(furi, obj, retain);
    };
    ////////////////////////////////////////////////////////////////////////////////////
    LOG_KERNEL_OBJ(INFO, this, "!yrouter!! started\n");
  }

  void Router::push_frame(const Pattern &pattern, const Rec_p &frame_data) {
    THREAD_FRAME_STACK = make_shared<Frame<>>(pattern, THREAD_FRAME_STACK, frame_data);
    //LOG_OBJ(TRACE, this, "!gpushed!! to frame stack [!mdepth!!: %i]: %s\n", THREAD_FRAME_STACK->depth(),
    //        THREAD_FRAME_STACK->full_frame()->toString().c_str());
  }

  void Router::pop_frame() {
    if(nullptr == THREAD_FRAME_STACK)
      throw fError("there are no more frames on the stack");
    THREAD_FRAME_STACK = THREAD_FRAME_STACK->previous;
  }


  void Router::loop() const {
    bool remove = false;
    for(Structure_p &s: *this->structures_) {
      if(!s->available())
        remove = true;
      else
        s->loop();
    }
    if(remove) {
      this->structures_->remove_if([this](const Structure_p &structure) {
        if(!structure->available()) {
          LOG_KERNEL_OBJ(INFO, this, FURI_WRAP " !y%s!! detached\n", structure->pattern()->toString().c_str(),
                         structure->tid_->name().c_str());
          return true;
        }
        return false;
      });
    }
  }

  void Router::stop() const {
    auto map = make_shared<Map<string, int>>();
    this->structures_->forEach([map](const Structure_p &structure) {
      const string name = structure->tid_->name();
      int count = map->count(name) ? map->at(name) : 0;
      count++;
      if(map->count(name))
        map->erase(name);
      map->insert({name, count});
    });
    for(const auto &[name, count]: *map) {
      LOG_KERNEL_OBJ(INFO, this, "!b%s !y%s!!(s) closing\n", to_string(count).c_str(), name.c_str());
    }
    this->structures_->forEach([](const Structure_p &structure) { structure->stop(); });
    LOG_KERNEL_OBJ(INFO, this, "!yrouter !b%s!! stopped\n", this->vid_->toString().c_str());
  }

  void Router::attach(const Structure_p &structure) const {
    if(structure->pattern()->equals(Pattern(""))) {
      LOG_KERNEL_OBJ(INFO, this, "!b%s !yempty structure!! ignored\n", structure->pattern()->toString().c_str(),
                     structure->tid_->name().c_str());
    } else {
      this->structures_->forEach([structure, this](const Structure_p &s) {
        if(structure->pattern()->bimatches(*s->pattern())) {
          // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
          throw fError(ROUTER_FURI_WRAP
                       " only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                       this->vid_->toString().c_str(), s->pattern()->toString().c_str(),
                       structure->pattern()->toString().c_str());
        }
      });
      this->structures_->push_back(structure);
      structure->setup();
      if(structure->available()) {
        LOG_KERNEL_OBJ(INFO, this, "!b%s!! !y%s!! attached\n", structure->pattern()->toString().c_str(),
                       structure->tid_->name().c_str());
      } else {
        LOG_KERNEL_OBJ(ERROR, this, "!runable to attach %s: %s!!\n", structure->pattern()->toString().c_str(),
                       structure->tid_->name().c_str());
        this->structures_->pop_back();
      }
    }
    this->save();
  }


  void Router::save() const {
    const Lst_p strcs = Obj::to_lst();
    this->structures_->forEach([strcs](const Structure_p &structure) { strcs->lst_add(vri(structure->pattern())); });
    this->rec_set("structure", strcs);
    Obj::save();
  }

  [[nodiscard]] Obj_p Router::exec(const ID_p &bcode_id, const Obj_p &arg) { return this->read(bcode_id)->apply(arg); }

  [[nodiscard]] Objs_p Router::read(const fURI_p &furi) {
    try {
      if(THREAD_FRAME_STACK) {
        const Obj_p frame_obj = THREAD_FRAME_STACK->read(furi);
        if(nullptr != frame_obj)
          return frame_obj;
      }
      const fURI_p resolved_furi = this->resolve(*furi);
      // const bool query = resolved_furi->has_query("structure");
      const Structure_p structure = this->get_structure(p_p(*resolved_furi));
      // if(query)
      //   return structure->shared_from_this();
      const Objs_p objs = structure->read(resolved_furi);
      LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_reading!! !g[!b%s!m=>!y%s!g]!! from " FURI_WRAP "\n",
                     this->vid_->toString().c_str(), resolved_furi->toString().c_str(), // make this the current process
                     objs->toString().c_str(), structure->pattern()->toString().c_str());
      return objs->none_one_all();
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
      return noobj();
    }
  }

  void Router::write(const fURI_p &furi, const Obj_p &obj, const bool retain) {
    try {
      const Structure_p structure = this->get_structure(p_p(*furi));
      LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_writing!! %s !g[!b%s!m=>!y%s!g]!! to " FURI_WRAP "\n",
                     Process::current_process()->vid_->toString().c_str(), retain ? "retained" : "transient",
                     furi->toString().c_str(), obj->tid_->toString().c_str(),
                     structure->pattern()->toString().c_str());
      structure->write(furi, obj, retain);
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
    }
  }

  void Router::unsubscribe(const ID_p &subscriber, const Pattern_p &pattern) {
    try {
      this->structures_->forEach([this, subscriber, pattern](const Structure_p &structure) {
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

  void Router::subscribe(const Subscription_p &subscription) {
    try {
      const Structure_p struc = this->get_structure(subscription->pattern());
      LOG_KERNEL_OBJ(DEBUG, this, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
    }
  }

  void *Router::import() {
    Router::singleton()->write(Router::singleton()->vid_, Router::singleton(),RETAIN);
    InstBuilder::build(Router::singleton()->vid_->extend("detach"))
        ->domain_range(URI_FURI, {1, 1}, NOOBJ_FURI, {0, 0})
        ->inst_f([](const Obj_p &lhs, const InstArgs &) {
          Router::singleton()->get_structure(p_p(lhs->uri_value()))->stop();
          return noobj();
        })->save();
    return nullptr;
  }

  Structure_p Router::get_structure(const Pattern_p &pattern, const bool throw_on_error) const {
    const Pattern_p temp = pattern->is_branch() ? p_p(pattern->extend("+")) : pattern;
    const List<Structure_p> list = this->structures_->find_all(
      [pattern, temp](const Structure_p &structure) {
        return pattern->matches(*structure->pattern()) || temp->matches(*structure->pattern());
      },
      true); // TODO: NO MUTEX!
    if(throw_on_error) {
      if(list.size() > 1)
        throw fError(ROUTER_FURI_WRAP " crosses multiple structures", pattern->toString().c_str());
      if(list.empty())
        throw fError(ROUTER_FURI_WRAP " has no structure for !b%s!!", this->vid_->toString().c_str(),
                     pattern->toString().c_str());
      return list.at(0);
    }
    const Structure_p s = list.size() == 1 ? list.at(0) : nullptr;
    return s;
  }


  [[nodiscard]] fURI_p Router::resolve(const fURI &furi) const {
    if(!furi.headless() && !furi.has_components())
      return furi_p(furi);
    List<fURI> components = furi.has_components() ? List<fURI>() : List<fURI>{furi};
    if(furi.has_components()) {
      for(const auto &c: furi.components()) {
        components.emplace_back(c);
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
        if(const Structure_p structure = this->get_structure(p_p(*x), false); structure && structure->has(x)) {
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
        test = furi_p(test->add_component(found ? *found : fURI(c)));
      }
    }
    return /*furi.has_query("domain") ? id_p(test->query(furi.query())) :*/ test;
  }
} // namespace fhatos
