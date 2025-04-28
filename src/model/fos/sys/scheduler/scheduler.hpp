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
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include "../../../../fhatos.hpp"
//
#include <atomic>
#include "../../../../furi.hpp"
#include "thread/thread.hpp"
#include "../../../../structure/router.hpp"
#include "../../../../util/mutex_deque.hpp"

namespace fhatos {
  // using Thread_p = ptr<Thread>;
  //static ID_p SCHEDULER_FURI = id_p("/sys/scheduler_t");

  class Scheduler final : public Rec {

  protected:
    Mutex mutex = Mutex();

  public:
    explicit Scheduler(const ID &id) :
      Rec(rmap({
              {"spawn", lst()},
              {"bundle", lst()}}),
          OType::REC, REC_FURI, id_p(id)) {
      SCHEDULER_ID = this->vid;
      LOG_WRITE(INFO, this, L("!g[!y{}!g] !yscheduler!! started\n", id.toString()));
    }

    static ptr<Scheduler> &singleton(const ID &id = ID("/sys/scheduler")) {
      static auto scheduler = std::make_shared<Scheduler>(id);
      return scheduler;
    }

    static bool shutting_down() {
      return ROUTER_READ(SCHEDULER_ID->extend("halt"))->or_else(dool(false))->bool_value();
    }

    void stop() {
      for(const Uri_p &bundle_uri: *this->rec_get("bundle")->lst_value()) {
        LOG_WRITE(INFO, this, L("!b{} !yfiber!! closing\n", bundle_uri->toString()));
        ROUTER_WRITE(bundle_uri->uri_value(), Obj::to_noobj(), true);
        Router::singleton()->loop();
      }
      std::vector<Uri_p> list = *this->rec_get("spawn")->lst_value();
      while(!list.empty()) {
        if(list.back()->is_uri()) {
          const Obj_p thread_obj = ROUTER_READ(list.back()->uri_value());
          list.pop_back();
          LOG_WRITE(INFO, this, L("!b{} !ythread!! closing\n", thread_obj->vid_or_tid()->toString()));
          thread_obj->obj_set("halt", dool(true));
        }
        Router::singleton()->loop();
      }
      const auto timestamp = std::chrono::system_clock::now();
      while((std::chrono::system_clock::now() - timestamp) < std::chrono::milliseconds(250)) {
        //do nothing (waiting for threads to close)
      }
      Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      LOG_WRITE(INFO, this, L("!yscheduler !b{}!! stopped\n", this->vid->toString()));
    }

    void loop() {
      try {
        this->handle_bundle();
        //this->handle_spawn();
      } catch(const fError &e) {
        LOG_WRITE(ERROR, this,L("scheduling error: {}", e.what()));
      }
    }

    void handle_memory() {
      // check memory and if low, warn logging.
    }

    /*void handle_spawn() {
      const Lst_p thread_uris = this->obj_get("thread")->clone()->or_else(lst());
      if(thread_uris->lst_value()->empty())
        return;
      const size_t count = thread_uris->lst_value()->size();
      thread_uris->lst_value()->erase(
          std::remove_if<>(thread_uris->lst_value()->begin(),
                           thread_uris->lst_value()->end(),
                           [this](const Uri_p &thread_id) -> bool {
                             if(!thread_id->is_uri()) {
                               LOG_WRITE(ERROR, this,L("scheduler thread can only store uris: {}",
                                                       OTypes.to_chars(thread_id->otype).c_str()));
                               return true;
                             }
                             if(ROUTER_READ(thread_id->uri_value())->is_noobj() ||
                                ROUTER_READ(thread_id->uri_value().extend("halt"))->or_else(dool(false))->
                                bool_value()) {
                               LOG_WRITE(INFO, this, L("!b{} !ythread!! removed\n", thread_id->uri_value().toString()));
                               return true;
                             }
                             return false;
                           }), thread_uris->lst_value()->end());
      if(thread_uris->lst_value()->size() != count)
        this->obj_set("thread", thread_uris);
    }*/

    void spawn_thread(const Obj_p &thread_obj) {
      if(!thread_obj->vid)
        throw fError("!ythread obj !rmust have!y a vid!!: %s", thread_obj->toString().c_str());
      auto lock = std::lock_guard<Mutex>(mutex);
      thread_obj->obj_set("halt", dool(false));
      const Lst_p thread_uris = Scheduler::singleton()->obj_get("spawn")->clone()->or_else(lst());
      /*for(const auto &id: *thread_uris->lst_value()) {
        if(thread_obj->vid->equals(id->uri_value()))
          throw fError("!g[!b%s!g] !ralready spawned!!: %s", thread_obj->vid->toString().c_str(),
                       thread_obj->toString().c_str());
      }*/
      thread_uris->lst_value()->push_back(Obj::to_uri(*thread_obj->vid));
      Scheduler::singleton()->obj_set("spawn", thread_uris);
      ROUTER_WRITE(thread_obj->vid->extend("halt").query("sub"), Subscription::create(
                       Scheduler::singleton()->vid, p_p(thread_obj->vid->extend("halt")),
                       [](const Obj_p &obj, const InstArgs &args) { // target = thread_id/halt
                         if(obj->is_bool() && obj->bool_value()) {
                           //LOG_WRITE(INFO,Scheduler::singleton().get(),L("HERE: {} {} -> {}\n", obj->toString(), Scheduler::singleton()->rec_get("thread")->toString(), args->arg("target")->toString()));
                           const Lst_p new_thread_ids = Obj::to_lst();
                           for(const Uri_p &thread_id:
                               *Scheduler::singleton()->obj_get("spawn")->or_else(lst())->lst_value()) {
                             if(!thread_id->uri_value().matches(args->arg("target")->uri_value().retract())) {
                               new_thread_ids->lst_add(thread_id);
                             } else {
                               ROUTER_WRITE(args->arg("target")->uri_value().query("sub"), Obj::to_noobj(), true);
                               LOG_WRITE(INFO, Scheduler::singleton().get(),
                                         L("!ythread !b{}!! unscheduled\n", thread_id->toString()));
                             }
                           }
                           Scheduler::singleton()->obj_set("spawn", new_thread_ids);
                         }
                         return Obj::to_noobj();
                       }), true);
      LOG_WRITE(INFO, Scheduler::singleton().get(), L("!b{} !ythread!! spawned\n", thread_obj->vid->toString()));
      const ptr<Thread> thread_state = Thread::get_state(thread_obj);
    }

    void bundle_fiber(const Obj_p &fiber_obj) {
      auto lock = std::lock_guard<Mutex>(mutex);
      const Lst_p fiber_uris = Scheduler::singleton()->obj_get("bundle")->clone()->or_else(lst());
      fiber_uris->lst_value()->push_back(Obj::to_uri(*fiber_obj->vid));
      Scheduler::singleton()->obj_set("bundle", fiber_uris);
      LOG_WRITE(INFO, Scheduler::singleton().get(), L("!b{} !yfiber!! bundled\n", fiber_obj->vid->toString()));
    }

    void handle_bundle() {
      auto lock = std::lock_guard<Mutex>(mutex);
      const Lst_p bundle_uris = this->obj_get("bundle")->clone()->or_else(lst());
      if(bundle_uris->lst_value()->empty())
        return;
      const size_t count = bundle_uris->lst_value()->size();
      bundle_uris->lst_value()->erase(
          std::remove_if<>(
              bundle_uris->lst_value()->begin(),
              bundle_uris->lst_value()->end(),
              [this](const Uri_p &fiber_id) -> bool {
                if(!fiber_id->is_uri()) {
                  LOG_WRITE(ERROR, this,
                            L("fiber bundles can only store uris: {}", OTypes.to_chars(fiber_id->otype).c_str()));
                  return true;
                }
                const Obj_p fiber = ROUTER_READ(fiber_id->uri_value());
                if(fiber->is_noobj()) {
                  LOG_WRITE(INFO, this, L("!b{} !yfiber!! removed\n", fiber_id->uri_value().toString()));
                  return true;
                }
                try {
                  const Inst_p fiber_loop_inst =
                      Compiler(true, true).resolve_inst(fiber, Obj::to_inst(Obj::to_inst_args(), id_p("loop")));
                  // LOG_WRITE(INFO, this,L("{}\n", fiber_loop_inst->toString()));
                  mmADT::delift(fiber_loop_inst)->apply(fiber);
                  return false;
                } catch(const fError &e) {
                  LOG_WRITE(ERROR, this,
                            L("!b{} !yfiber !rloop error!!: {}\n", fiber->vid_or_tid()->toString(), e.what()));
                  return true;
                }
              }), bundle_uris->lst_value()->end());
      if(bundle_uris->lst_value()->size() != count)
        this->obj_set("bundle", bundle_uris);
    }

    static void *import() {
      if(const Rec_p config = ROUTER_READ(FOS_BOOT_CONFIG_VALUE_ID);
        !config->is_noobj())
        Scheduler::singleton()->rec_set("config", config->rec_get("scheduler")->or_else(noobj()));
      Scheduler::singleton()->save();
      InstBuilder::build(Scheduler::singleton()->vid->add_component("spawn"))
          ->inst_args(rec({{"thread", Obj::to_bcode()}}))
          ->domain_range(OBJ_FURI, {0, 1}, THREAD_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p thread_obj = args->arg("thread");
            Scheduler::singleton()->spawn_thread(thread_obj);
            return thread_obj;
          })
          ->save();
      InstBuilder::build(Scheduler::singleton()->vid->add_component("bundle"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_args(rec({{"fiber", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p fiber = args->arg("fiber");
            Scheduler::singleton()->bundle_fiber(fiber);
            return fiber;
          })
          ->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
