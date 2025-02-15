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
#include "../util/print_helper.hpp"
#include "stype/heap.hpp"
#include STR(stype/mqtt/HARDWARE/mqtt.hpp)

namespace fhatos {
  inline thread_local ptr<Frame<>> THREAD_FRAME_STACK = nullptr;

  ptr<Router> Router::singleton(const ID &value_id) {
    static auto router_p = std::make_shared<Router>(value_id);
    return router_p;
  }

  void Router::log_frame_stack(const LOG_TYPE log_type) const {
    if(nullptr != THREAD_FRAME_STACK) {
      int counter = 0;
      ptr<Frame<>> frame = THREAD_FRAME_STACK;
      while(nullptr != frame) {
        LOG_OBJ(log_type, frame, "!m%s!g>!! %s\n", StringHelper::repeat(++counter,"-").c_str(),
                frame->toString().c_str());
        frame = frame->previous;
      }
    }
  }

  Router::Router(const ID &id) :
    Rec(rmap({{"structure", to_lst()}}),
        //stop and attach
        OType::REC, REC_FURI, id_p(id)),
    structures_(make_unique<MutexDeque<Structure_p>>()) {
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_ID = id_p(this->vid);
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_PUSH_FRAME = [this](const Pattern &pattern, const Rec_p &frame_data) {
      this->push_frame(pattern, frame_data);
    };
    ROUTER_POP_FRAME = [this] {
      this->pop_frame();
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_RESOLVE = [this](const fURI &furi) -> fURI {
      return this->resolve(furi);
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_READ = [this](const fURI &furi) -> Obj_p {
      return this->read(furi);
    };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_WRITE = [this](const fURI &furi, const Obj_p &obj, const bool retain) {
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

  void Router::load_config(const ID &config_id) {
    const Obj_p config = this->read(config_id);
    if(config->is_noobj())
      LOG_KERNEL_OBJ(WARN, this, "!b%s!! does not reference a config obj\n", config_id.toString().c_str());
    if(!config->is_noobj()) {
      const Rec_p router_config = config->rec_get(vri("router"));
      this->rec_set("config", router_config);
    }
  }


  void Router::loop() {
    bool remove = false;
    for(const Structure_p &s: *this->structures_) {
      if(!s->available())
        remove = true;
      else
        s->loop();
    }
    if(remove) {
      this->structures_->remove_if([this](const Structure_p &structure) {
        if(!structure->available()) {
          LOG_KERNEL_OBJ(INFO, this, "!b%s !y%s!! detached\n", structure->pattern->toString().c_str(),
                         structure->tid->name().c_str());
          return true;
        }
        return false;
      });
      this->save();
    }
    //this->load();
  }

  void Router::stop() {
    auto map = make_shared<Map<string, int>>();
    this->structures_->forEach([map](const Structure_p &structure) {
      const string name = structure->tid->name();
      int count = map->count(name) ? map->at(name) : 0;
      count++;
      if(map->count(name))
        map->erase(name);
      map->insert({name, count});
    });
    for(const auto &[name, count]: *map) {
      LOG_KERNEL_OBJ(INFO, this, "!b%s !y%s!!(s) closing\n", to_string(count).c_str(), name.c_str());
    }
    this->active = false;
    this->structures_->forEach([](const Structure_p &structure) { structure->stop(); });
    LOG_KERNEL_OBJ(INFO, this, "!yrouter !b%s!! stopped\n", this->vid->toString().c_str());
  }

  void Router::attach(const Structure_p &structure) const {
    if(structure->pattern->equals(Pattern(""))) {
      LOG_KERNEL_OBJ(INFO, this, "!b%s !yempty structure!! ignored\n", structure->pattern->toString().c_str(),
                     structure->tid->name().c_str());
    } else {
      this->structures_->forEach([structure, this](const Structure_p &s) {
        if(structure->pattern->bimatches(*s->pattern)) {
          // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
          throw fError(ROUTER_FURI_WRAP
                       " only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                       this->vid->toString().c_str(), s->pattern->toString().c_str(),
                       structure->pattern->toString().c_str());
        }
      });
      this->structures_->push_back(structure);
      structure->setup();
      if(structure->available()) {
        LOG_KERNEL_OBJ(INFO, this, "!y%s !b%s !yspanning !b%s!! attached\n",
                       structure->tid->name().c_str(),
                       structure->vid ? structure->vid->toString().c_str() : "<none>",
                       structure->pattern->toString().c_str());
      } else {
        LOG_KERNEL_OBJ(ERROR, this, "!runable to attach!! %s: %s at %s!!\n", structure->pattern->toString().c_str(),
                       structure->tid->name().c_str(),
                       structure->vid ? structure->vid->toString().c_str() : "<none>");
        this->structures_->pop_back();
      }
    }
    this->save();
  }


  void Router::save() const {
    const Lst_p strc = Obj::to_lst();
    this->structures_->forEach([strc](const Structure_p &structure) { strc->lst_add(vri(structure->pattern)); });
    this->rec_set(FOS_ROUTER_STRUCTURE, strc);
    Obj::save();
  }

  [[nodiscard]] Obj_p Router::exec(const ID &bcode_id, const Obj_p &arg) { return this->read(bcode_id)->apply(arg); }

  //[[nodiscard]] Objs_p Router::read(const vID &variant) {
  //  return this->read(furi_p(variant.as_()));
  // }

  [[nodiscard]] Objs_p Router::read(const fURI &furi) {
    if(!this->active)
      return Obj::to_noobj();
    try {
      if(THREAD_FRAME_STACK) {
        if(const Obj_p frame_obj = THREAD_FRAME_STACK->read(furi);
          nullptr != frame_obj)
          return frame_obj;
      }
      const fURI resolved_furi = this->resolve(furi);
      const Structure_p structure = this->get_structure(resolved_furi);
      const Objs_p objs = structure->read(resolved_furi);
      LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_reading!! !g[!b%s!m=>!y%s!g]!! from " FURI_WRAP "\n",
                     this->vid->toString().c_str(), resolved_furi.toString().c_str(), // make this the current process
                     objs->toString().c_str(), structure->pattern->toString().c_str());
      return objs->none_one_all();
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
      return noobj();
    }
  }

  void Router::write(const fURI &furi, const Obj_p &obj, const bool retain) {
    if(!this->active)
      return;
    if(obj->is_noobj() && furi.is_node() && this->vid->matches(furi))
      this->active = false;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////// PROCESS FURI QUERIES  ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(furi.has_query()) {
      for(const auto &[query_key, query_value]: furi.query_values()) {
        if(query_key != "sub") {
          const Obj_p query_processor = this->read(this->vid->extend(FOS_ROUTER_QUERY_WRITE).extend(query_key));
          if(query_processor->is_noobj())
            throw fError("router has no query processor for !y%s!!", query_key.c_str());
          query_processor->apply(obj, Obj::to_inst_args({vri(furi)}));
        }
      }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    try {
      const Structure_p structure = this->get_structure(furi, obj);
      LOG_KERNEL_OBJ(DEBUG, this, FURI_WRAP " !g!_writing!! %s !g[!b%s!m=>!y%s!g]!! to " FURI_WRAP "\n",
                     Process::current_process()->vid->toString().c_str(), retain ? "retained" : "transient",
                     furi.toString().c_str(), obj->tid->toString().c_str(),
                     structure->pattern->toString().c_str());
      structure->write(furi, obj, retain);
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
    }
    if(!this->active)
      this->stop();
    /*if(furi->matches(this->vid->extend("#")))
      this->stale = true;*/
  }

  void Router::unsubscribe(const ID &subscriber, const fURI &pattern) {
    if(!this->active)
      return;
    try {
      this->structures_->forEach([this, subscriber, pattern](const Structure_p &structure) {
        if(structure->pattern->matches(pattern) || pattern.matches(*structure->pattern)) {
          LOG_KERNEL_OBJ(DEBUG, this, "!y!_routing unsubscribe!! !b%s!! for %s\n", pattern.toString().c_str(),
                         subscriber.toString().c_str());
          structure->recv_unsubscribe(subscriber, pattern);
        }
      });
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
    }
  }

  void Router::subscribe(const Subscription_p &subscription) {
    if(!this->active)
      return;
    try {
      const Structure_p struc = this->get_structure(*subscription->pattern());
      LOG_KERNEL_OBJ(DEBUG, this, "!y!_routing subscribe!! %s\n", subscription->toString().c_str());
      struc->recv_subscription(subscription);
    } catch(const fError &e) {
      LOG_EXCEPTION(this->shared_from_this(), e);
    }
  }

  void *Router::import() {
    Router::singleton()->write(*Router::singleton()->vid, Router::singleton(),RETAIN);
    Router::singleton()->write(FRAME_FURI, Obj::to_type(REC_FURI),RETAIN);
    Router::singleton()->load_config(FOS_BOOT_CONFIG_VALUE_ID);
    Router::singleton()->write(Router::singleton()->vid->retract().extend("lib/msg"),
                               Obj::to_rec({{"target", Obj::to_type(URI_FURI)},
                                            {"payload", Obj::to_bcode()},
                                            {"retain", Obj::to_type(BOOL_FURI)}}));
    Router::singleton()->write(Router::singleton()->vid->retract().extend("lib/sub"),
                               Obj::to_rec({{"source", Obj::to_type(URI_FURI)},
                                            {"pattern", Obj::to_type(URI_FURI)},
                                            {":on_recv", Obj::to_bcode()}}));
    /*  InstBuilder::build(Router::singleton()->vid->extend(":detach"))
          ->domain_range(URI_FURI, {0, 1}, NOOBJ_FURI, {0, 0})
          ->type_args(x(0, ___()))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            Router::singleton()->get_structure(p_p(args->arg(0)->uri_value()))->stop();
            return noobj();
          })->save();*/
    /* InstBuilder::build(Router::singleton()->vid->extend(":stop"))
         ->domain_range(OBJ_FURI, {0, 1}, NOOBJ_FURI, {0, 0})
         ->inst_f([](const Obj_p &, const InstArgs &args) {
           Router::singleton()->stop();
           return Obj::to_noobj();
         })->save();*/
    /// query extensions
    InstBuilder::build(Router::singleton()->vid->extend(FOS_ROUTER_QUERY_WRITE).extend("lock"))
        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
        ->inst_f([](const Obj_p &, const InstArgs &args) {
          const Obj_p obj = args->arg(0);
          if(obj->lock().has_value() && !obj->lock().value().equals(
                 *Process::current_process()->vid)) {
            throw fError("!runable write obj!! locked by !b%s!!: %s",
                         obj->lock().value().toString().c_str(),
                         obj->toString().c_str());
          }
          return obj;
        })->save();
    InstBuilder::build(Router::singleton()->vid->extend(FOS_ROUTER_QUERY_WRITE).extend("sub"))
        ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
        ->inst_f([](const Obj_p &obj, const InstArgs &args) {
          LOG(ERROR, "sub query processor to be implemented\n");
          return obj;
        })->save();
    //  Router::singleton()->load();
    return nullptr;
  }

  Structure_p Router::get_structure(const Pattern &pattern, const Obj_p &to_write, const bool throw_on_error) const {
    // const Pattern_p temp = pattern->is_branch() ? p_p(pattern->extend("+")) : pattern;
    Structure_p found = nullptr;
    for(const Structure_p &s: *this->structures_) {
      if(to_write && to_write->is_noobj() && s->vid && pattern.bimatches(*s->vid)) {
        s->stop();
      } else {
        if(pattern.bimatches(*s->pattern)) {
          if(found && throw_on_error)
            throw fError("!b%s!! crosses multiple structures", pattern.toString().c_str());
          found = s;
        }
      }
    }
    if(!found && throw_on_error) { // && !pattern->empty() ??
      const Lst_p related = Obj::to_lst();
      const fURI sub_pattern = pattern.retract_pattern();
      for(const auto &s: *this->structures_) {
        if(sub_pattern.is_subfuri_of(*s->pattern)) {
          related->lst_add(vri(s->pattern));
        }
      }
      throw fError("!rno attached structure!! for !b%s!! %s %s",
                   pattern.toString().c_str(),
                   related->lst_value()->empty() ? "" : "\n" FOS_TAB_2 "!yavailable !bsub-structures!!:",
                   related->lst_value()->empty() ? "" : PrintHelper::pretty_print_obj(related, 1).c_str());
    }
    return found ? found : nullptr;
  }

  [[nodiscard]] fURI Router::resolve(const fURI &furi) const {
    if(!this->active)
      return furi;
    if(furi.empty())
      return furi;
    if(const Structure_p structure = this->get_structure(furi, nullptr, false); structure && structure->has(furi))
      return furi;
    if(!furi.headless() && !furi.has_components())
      return furi;
    List<fURI> components = furi.has_components() ? List<fURI>() : List<fURI>{furi};
    if(furi.has_components()) {
      for(const auto &c: furi.components()) {
        components.emplace_back(c);
      }
    }
    bool first = true;
    fURI_p test = nullptr;
    for(const auto &c: components) {
      List_p<Uri_p> prefixes = this->rec_get("config/resolve/auto_prefix")->or_else(lst())->lst_value();
      if(prefixes->empty())
        LOG_KERNEL_OBJ(WARN, this, "router has no auto-prefix configuration: %s\n",
                     this->rec_get("config")->toString().c_str());
      // TODO: make this an exposed property of /sys/router
      fURI_p found = nullptr;
      for(const auto &prefix: *prefixes) {
        const fURI x = prefix->uri_value().extend(c);
        if(const Structure_p structure = this->get_structure(x, nullptr, false); structure && structure->has(x)) {
          LOG_KERNEL_OBJ(TRACE, this, "located !b%s!! in %s and resolved to !b%s!!\n",
                         furi.toString().c_str(),
                         structure->toString().c_str(),
                         x.toString().c_str());
          found = furi_p(x);
          break;
        }
      }
      if(!found) {
        LOG_KERNEL_OBJ(TRACE, this, "unable to locate !b%s!!\n", c.toString().c_str());
      }
      if(first) {
        first = false;
        test = furi_p(found ? *found : c);
      } else {
        test = furi_p(test->add_component(found ? *found : c));
      }
    }
    return /*furi.has_query("domain") ? id_p(test->query(furi.query())) :*/ *test;
  }
} // namespace fhatos
