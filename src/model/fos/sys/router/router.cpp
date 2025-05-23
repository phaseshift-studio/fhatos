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
#include "../../../../util/obj_helper.hpp"
#include "../../../../util/print_helper.hpp"
#include "../../s/frame.hpp"

namespace fhatos {

  inline thread_local ptr<Frame<>> THREAD_FRAME_STACK = nullptr;

  ptr<Router> &Router::singleton(const ID &vid) {
    static auto router = std::make_shared<Router>(vid);
    if(BOOTING && !router->vid->equals(vid) && router->vid->path().find("boot") != std::string::npos) {
      router->vid = id_p(vid);
      ROUTER_ID = router->vid;
      // router->save();
      LOG_WRITE(INFO, router.get(), L("!grouter!! !bid!! reassigned\n"));
    }
    return router;
  }

  ptr<Frame<>> Router::get_frame() { return THREAD_FRAME_STACK; }

  Router::Router(const ID &id) :
      Rec(rmap({{"structure", to_lst()}}),
          // stop and attach
          OType::REC, REC_FURI, id_p(id)),
      structures_(make_shared<MutexDeque<Structure_p>>()) {
    load_logger();
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_ID = id_p(id);
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_PUSH_FRAME = [this](const Pattern &pattern, const Rec_p &frame_data) {
      fhatos::Router::push_frame(pattern, frame_data);
    };
    ROUTER_POP_FRAME = [this] { this->pop_frame(); };
    ROUTER_GET_FRAME_DATA = [this] { return this->get_frame()->full_frame(); };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_RESOLVE = [this](const fURI &furi) -> fURI { return this->resolve(furi); };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_READ = [this](const fURI &furi) -> Obj_p { return this->read(furi); };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_WRITE = [this](const fURI &furi, const Obj_p &obj, const bool retain) { this->write(furi, obj, retain); };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_APPEND = [this](const fURI &furi, const Obj_p &obj) { this->append(furi, obj); };
    ////////////////////////////////////////////////////////////////////////////////////
    ROUTER_EXEC_WITHIN_FRAME = [this](const Pattern &span, const Rec_p &frame, const Supplier<Obj_p> &to_exec) {
      bool frame_set = false;
      Obj_p result = nullptr;
      try {
        Router::push_frame(span, frame);
        frame_set = true;
        result = to_exec();
      } catch(std::exception &) {
        if(frame_set)
          Router::pop_frame();
        throw;
      }
      this->pop_frame();
      return result;
    };
    LOG_WRITE(INFO, this, L("!yrouter!! started\n"));
  }

  void Router::push_frame(const Pattern &pattern, const Rec_p &frame_data) {
    THREAD_FRAME_STACK = make_shared<Frame<>>(pattern, THREAD_FRAME_STACK, frame_data);
  }

  void Router::pop_frame() {
    if(nullptr == THREAD_FRAME_STACK)
      throw fError("there are no more frames on the stack");
    THREAD_FRAME_STACK = THREAD_FRAME_STACK->previous_;
  }

  void Router::loop() const {
    const uint8_t size = this->structures_->size();
    this->structures_->remove_if([this](const Structure_p &s) {
      if(!s->available()) {
        LOG_WRITE(INFO, this, L("!b{} !y{}!! detached\n", s->pattern->toString(), s->tid->name()));
        return true;
      } else {
        s->loop();
        return false;
      }
    });
    if(this->structures_->size() != size)
      this->save();
  }

  void Router::stop() {
    auto map = make_shared<Map<string, int>>();
    this->structures_->forEach([&map](const Structure_p &structure) {
      const string name = structure->tid->name();
      int count = map->count(name) ? map->at(name) : 0;
      count++;
      if(map->count(name))
        map->erase(name);
      map->insert({name, count});
    });
    for(const auto &[name, count]: *map) {
      LOG_WRITE(INFO, this, L("!b{} !y{}!!(s) closing\n", to_string(count), name));
    }
    this->structures_->forEach([](const Structure_p &structure) {
      if(structure->available()) {
         structure->stop();
      }
    });
    // while(!this->structures_->empty()) {
    // std::optional<Structure_p> op = this->structures_->pop_back();
    // if(op.has_value()) {
    // op.value()->stop(); TODO: why seg fault?
    //  }
    // }
    LOG_WRITE(INFO, this, L("!yrouter !b{}!! stopped\n", this->vid->toString()));
  }

  void Router::attach(const Structure_p &structure) const {
    if(structure->pattern->equals(Pattern(""))) {
      LOG_WRITE(INFO, this,
                L("!b{} !yempty structure!! ignored\n", structure->pattern->toString(), structure->tid->name()));
    } else {
      this->structures_->forEach([structure, this](const Structure_p &s) {
        if(structure->pattern->bimatches(*s->pattern)) {
          // symmetric check necessary as A can't be a subpattern of B and B can't be a subpattern of A
          throw fError(ROUTER_FURI_WRAP " only !ydisjoint structures!! can coexist: !g[!b%s!g]!! overlaps !g[!b%s!g]!!",
                       this->vid->toString().c_str(), s->pattern->toString().c_str(),
                       structure->pattern->toString().c_str());
        }
      });
      this->structures_->push_back(structure);
      structure->setup();
      if(structure->available()) {
        LOG_WRITE(INFO, this,
                  L("!y{} !b{} !yspanning !b{}!! mounted\n", structure->tid->name(),
                    structure->vid ? structure->vid->toString().c_str() : "<none>", structure->pattern->toString()));
        this->save();
      } else {
        LOG_WRITE(ERROR, this,
                  L("!runable to mount!! {}: {} at {}!!\n", structure->pattern->toString(), structure->tid->name(),
                    structure->vid ? structure->vid->toString() : "<none>"));
        this->structures_->pop_back();
      }
    }
  }


  void Router::save() const {
    const Lst_p structures = Obj::to_lst();
    this->structures_->forEach(
        [structures](const Structure_p &structure) { structures->lst_add(vri(structure->pattern)); });
    this->rec_set(FOS_ROUTER_STRUCTURE, structures);
    Obj::save();
  }

  [[nodiscard]] Objs_p Router::read(const fURI &furi) const {
    try {
      if(THREAD_FRAME_STACK) {
        if(const Obj_p frame_obj = THREAD_FRAME_STACK->read(furi); nullptr != frame_obj)
          return frame_obj;
      }
      const fURI resolved_furi = this->resolve(furi);
      const Structure_p structure = this->get_structure(resolved_furi);
      return structure ? structure->read(resolved_furi)->none_one_all() : Obj::to_noobj();
    } catch(const fError &e) {
      LOG_WRITE(BOOTING ? WARN : ERROR, this, L("{}\n", e.what()));
      return noobj();
    }
  }

  void Router::append(const fURI &furi, const Obj_p &obj) const {
    if(obj->is_noobj())
      return;
    try {
      if(const Structure_p structure = this->get_structure(furi, obj))
        structure->append(furi, obj);
    } catch(const fError &e) {
      LOG_WRITE(BOOTING ? WARN : ERROR, this, L("{}\n", e.what()));
    }
  }

  void Router::write(const fURI &furi, const Obj_p &obj, const bool retain) {
    if(obj->is_noobj() && furi.is_node() && this->vid->matches(furi))
      this->stop();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    try {
      if(const Structure_p structure = this->get_structure(furi, obj))
        structure->write(furi, obj, retain);
    } catch(const fError &e) {
      if(!BOOTING)
        throw;
      LOG_WRITE(WARN, this, L("{}\n", e.what()));
    }
  }

  void *Router::import() {
    Router::singleton()->auto_prefixes_ =
        std::vector<Uri_p>(*Router::singleton()->rec_get("config/auto_prefix")->or_else(lst())->lst_value());
    Router::singleton()->rec_set(
        "::/mount",
        InstBuilder::build(Router::singleton()->vid->add_component("mount"))
            ->inst_args(rec({{"structure", Obj::to_bcode()}}))
            ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
            ->inst_f([](const Obj_p &, const InstArgs &args) {
              const Obj_p structure_obj = args->arg("structure");
              const auto s = ptr<Structure>(structure_obj->get_model<Structure>());
              Router::singleton()->attach(s);
              Subscription::create(Router::singleton()->vid, p_p(*s->vid), [](const Obj_p &obj, const InstArgs &args) {
                if(obj->is_noobj()) {
                  const ID s_id = args->arg("target")->uri_value();
                  const Obj_p s_obj = Router::singleton()->read(s_id);
                  s_obj->get_model<Structure>()->stop();
                }
                return Obj::to_noobj();
              })->post();
              return s;
            })
            ->create());
    /* InstBuilder::build(Router::singleton()->vid->extend(":stop"))
            ->domain_range(OBJ_FURI, {0, 1}, NOOBJ_FURI, {0, 0})
            ->inst_f([](const Obj_p &, const InstArgs &args) {
              Router::singleton()->stop();
              return Obj::to_noobj();
            })->save();*/
    /// query extensions
    /*InstBuilder::build(Router::singleton()->vid->extend(FOS_ROUTER_QUERY_WRITE).extend("lock"))
        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
        ->inst_f([](const Obj_p &, const InstArgs &args) {
          const Obj_p obj = args->arg(0);
          if(obj->lock().has_value() &&
            \\!\\obj->lock().value().equals(Thread::current_thread().has_value()
                                             ? *Thread::current_thread().value()->thread_obj_->vid
                                             : *obj->vid)) {
            throw fError("!runable write obj!! locked by !b%s!!: %s", obj->lock().value().toString().c_str(),
                         obj->toString().c_str());
          }
          return obj;
        })
        ->save();
    InstBuilder::build(Router::singleton()->vid->extend(FOS_ROUTER_QUERY_WRITE).extend("sub"))
        ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
        ->inst_f([](const Obj_p &obj, const InstArgs &args) {
          LOG_WRITE(ERROR, Router::singleton().get(), L("sub query processor to be implemented\n"));
          return obj;
        })
        ->save();*/
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
        if(const auto p = Pattern(*s->pattern); pattern.bimatches(p)) {
          if(found && throw_on_error)
            throw fError("!b%s!! crosses multiple structures", pattern.toString().c_str());
          found = s;
        }
      }
    }
    if(!found && throw_on_error && !pattern.empty()) { // && !pattern->empty() ??
      const Lst_p related = Obj::to_lst();
      const fURI sub_pattern = pattern.retract_pattern();
      for(const auto &s: *this->structures_) {
        if(sub_pattern.is_subfuri_of(*s->pattern)) {
          related->lst_add(vri(s->pattern));
        }
      }
      throw fError("!rno mounted structure!! for !b%s!! %s %s", pattern.toString().c_str(),
                   related->lst_value()->empty() ? "" : "\n" FOS_TAB_2 "!yavailable !bsub-structures!!:",
                   related->lst_value()->empty() ? "" : PrintHelper::pretty_print_obj(related, 1).c_str());
    }
    return found ? found : nullptr;
  }

  [[nodiscard]] fURI Router::resolve(const fURI &furi) const {
    if(furi.empty() || (!furi.headless() && !furi.has_components()))
      return furi;
    fURI_p test = nullptr;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// START MUTEX RANGE
    {
      static auto mutex = Mutex();
      auto lock = shared_lock<Mutex>(mutex);
      if(const Structure_p structure = this->get_structure(furi, nullptr, false); structure && structure->has(furi))
        return furi;
      std::vector<fURI> components = furi.has_components() ? std::vector<fURI>() : std::vector<fURI>{furi};
      if(components.empty()) {
        for(const auto &c: furi.components()) {
          components.emplace_back(c);
        }
      }
      bool first = true;

      for(const auto &c: components) {
        if(this->auto_prefixes_.empty() && !BOOTING)
          LOG_WRITE(WARN, this, L("!bauto_prefix !ynot configured!!: {}\n", this->rec_get("config")->toString()));
        // TODO: make this an exposed property of /sys/router
        fURI_p found = nullptr;
        for(const auto &prefix: this->auto_prefixes_) {
          const fURI x = prefix->uri_value().extend(c);
          if(const Structure_p structure = this->get_structure(x, nullptr, false); structure && structure->has(x)) {
            // LOG_WRITE(TRACE, this, L("located !b{}!! in {} and resolved to !b{}!!\n",
            //                          furi.toString(), structure->toString(), x.toString()));
            found = furi_p(x);
            break;
          }
        }
        /*if(!found) {
          LOG_WRITE(TRACE, this, L("unable to locate !b{}!!\n", c.toString()));
        }*/
        if(first) {
          first = false;
          test = furi_p(found ? *found : c);
        } else {
          test = furi_p(test->add_component(found ? *found : c));
        }
      }
    }
    /// END MUTEX RANGE
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(furi.has_query(FOS_DOMAIN) && furi.has_query(FOS_RANGE)) {
      const fURI::DomainRange dm = furi.dom_rng();
      test = Compiler::generate_domain_range_type(test->no_query(), this->resolve(std::get<0>(dm)), std::get<1>(dm),
                                                  this->resolve(std::get<2>(dm)), std::get<3>(dm));
    }
    return *test;
  }
} // namespace fhatos
