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
  using Thread_p = ptr<Thread>;
  static ID_p SCHEDULER_FURI = id_p("/sys/scheduler_t");

  class Scheduler final : public Rec {
  protected:
    bool running_ = true;

  public:
    explicit Scheduler(const ID &id) :
      Rec(rmap({
              {"thread", lst()},
              {"bundle", lst()}}),
          OType::REC, REC_FURI, id_p(id)) {
      SCHEDULER_ID = this->vid;
      LOG_WRITE(INFO, this, L("!g[!y{}!g] !yscheduler!! started\n", id.toString()));
    }

    static ptr<Scheduler> singleton(const ID &id = ID("/sys/scheduler")) {
      static auto scheduler = std::make_shared<Scheduler>(id);
      return scheduler;
    }

    void stop() {
      for(const Uri_p &bundle_uri: *this->rec_get("bundle")->lst_value()) {
        LOG_WRITE(INFO, this, L("!b%s !yfiber!! closing\n", bundle_uri->toString()));
        ROUTER_WRITE(bundle_uri->uri_value(), Obj::to_noobj(), true);
      }
      std::vector<Uri_p> list = *this->rec_get("thread")->lst_value();
      while(!list.empty()) {
        if(list.back()->is_uri()) {
          const ptr<Thread> thread_state = Thread::get_state(list.back()->uri_value());
          list.pop_back();
          LOG_WRITE(INFO, this, L("!b%s !ythread!! closing\n", thread_state->thread_obj_->vid_or_tid()->toString()));
          if(!thread_state->thread_obj_->get<bool>("halt"))
            thread_state->halt();
          Thread::yield_current_thread();
        }
      }
      Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      this->running_ = false;
      LOG_WRITE(INFO, this, L("!yscheduler !b{}!! stopped\n", this->vid->toString()));
    }

    void barrier(const string &, const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG_WRITE(INFO, this, L("!mbarrier start: <!y{}!m>!!\n", "main"));
      if(message)
        LOG_WRITE(INFO, this, L("{}\n", message));

      //this->save();
      while(true) {
        try {
          this->handle_bundle();
          this->handle_threads();
          Router::singleton()->loop();
        } catch(const std::exception &e) {
          LOG_WRITE(ERROR, this,L("scheduling error: {}", e.what()));
        }
      }
      LOG_WRITE(INFO, this, L("!mbarrier end: <!g{}!m>!!\n", "main"));
    }

    void handle_threads() {
      this->sync("thread");
      const Lst_p thread_uris = this->rec_get("thread")->or_else(lst());
      if(thread_uris->lst_value()->empty())
        return;
      const auto new_thread_uris = Obj::to_lst();
      for(const auto &thread_id: *thread_uris->lst_value()) {
        const Bool_p halted = ROUTER_READ(thread_id->uri_value().extend("halt"));
        if(halted->is_noobj() || halted->bool_value()) {
          LOG_WRITE(INFO, this, L("!b{} !ythread!! removed\n", thread_id->uri_value().toString()));
          continue;
        }
        new_thread_uris->lst_add(thread_id);
        FEED_WATCHDOG();
      }
      if(!new_thread_uris->equals(*thread_uris)) {
        this->rec_set("thread", new_thread_uris);
        this->save("thread");
      }
    }

    void handle_bundle() {
      this->sync("bundle");
      const Lst_p bundle_uris = this->rec_get("bundle");
      if(bundle_uris->lst_value()->empty())
        return;
      const auto new_bundles = Obj::to_lst();
      for(const auto &fiber_id: *bundle_uris->lst_value()) {
        const Obj_p fiber = ROUTER_READ(fiber_id->uri_value());
        if(fiber->is_noobj()) {
          LOG_WRITE(INFO, this, L("!b{} !yfiber!! removed\n", fiber_id->uri_value().toString()));
          continue;
        }
        FEED_WATCHDOG();
        try {
          //__(fiber).inst("loop").compute();
          mmADT::delift(fiber->rec_get("loop"))->apply(fiber);
          new_bundles->lst_add(fiber_id);
        } catch(const std::exception &e) {
          LOG_WRITE(ERROR, this, L("!b{} !yfiber !rloop error!!: {}\n", fiber->vid_or_tid()->toString(), e.what()));
        }
      }
      if(!new_bundles->equals(*bundle_uris)) {
        this->rec_set("bundle", new_bundles);
        this->save("bundle");
      }
    }

    static void *import() {
      /* Typer::singleton()->save_type(*SCHEDULER_FURI,
                                     Obj::to_rec({
                                         {"thread", Obj::to_lst()},
                                         {"bundle", Obj::to_lst()}}));*/
      if(const Rec_p config = ROUTER_READ(FOS_BOOT_CONFIG_VALUE_ID);
        !config->is_noobj())
        Scheduler::singleton()->rec_set("config", config->rec_get("scheduler")->or_else(noobj()));
      Scheduler::singleton()->save();
      InstBuilder::build(Scheduler::singleton()->vid->add_component("spawn"))
          ->inst_args(rec({{"thread", Obj::to_bcode()}}))
          ->domain_range(OBJ_FURI, {0, 1}, THREAD_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p thread_obj = args->arg("thread");
            thread_obj->rec_set("halt", dool(false));
            thread_obj->save("halt");
            const Thread_p thread_state = Thread::get_state(thread_obj);
            const Lst_p thread_uris = Scheduler::singleton()->rec_get("thread")->or_else(lst());
            thread_uris->lst_value()->push_back(Obj::to_uri(*thread_obj->vid));
            Scheduler::singleton()->rec_value()->insert_or_assign(vri("thread"), thread_uris);
            Scheduler::singleton()->save("thread");
            LOG_WRITE(INFO, Scheduler::singleton().get(), L("!b{} !ythread!! spawned\n", thread_obj->vid->toString()));
            return thread_state->thread_obj_;
          })
          ->save();
      InstBuilder::build(Scheduler::singleton()->vid->add_component("bundle"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_args(rec({{"fiber", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p fiber = args->arg("fiber");
            const Lst_p bundle = Scheduler::singleton()->rec_get("bundle")->or_else(lst());
            bundle->lst_value()->push_back(Obj::to_uri(*fiber->vid));
            Scheduler::singleton()->rec_value()->insert_or_assign(vri("bundle"), bundle);
            Scheduler::singleton()->save("bundle");
            LOG_WRITE(INFO, Scheduler::singleton().get(), L("!b{} !yfiber!! bundled\n", fiber->vid->toString()));
            return fiber;
          })
          ->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
