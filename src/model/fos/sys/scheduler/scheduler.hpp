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

  class Scheduler final : public Rec {
  protected:
    const uptr<MutexDeque<Thread_p>> threads_ = std::make_unique<MutexDeque<Thread_p>>();
    bool running_ = true;
    ptr<Router> router_ = nullptr;

  public:
    explicit Scheduler(const ID &id) : Rec(rmap({{"thread", lst()}}),
                                           OType::REC, REC_FURI, id_p(id)) {
      SCHEDULER_ID = id_p(id);
      FEED_WATCHDOG = [this] {
        this->feed_local_watchdog();
      };
      LOG_WRITE(INFO, this, L("!g[!y/sys/scheduler!g] !yscheduler!! started\n"));
      // TODO: broadcast when online to trigger bootstrap of other models
      /*Router::singleton()->write(Router::singleton()->vid, vri(this->vid), false);
      Router::singleton()->route_subscription(Subscription::create(
        this->vid,
        p_p(*Router::singleton()->vid),
        to_inst("on_recv", to_rec({{"msg", to_bcode()}}), [](const Obj_p &lhs, const InstArgs &args) {
          return to_noobj();
        })));*/
    }

    static ptr<Scheduler> singleton(const ID &id = ID("/sys/scheduler")) {
      static auto scheduler = std::make_shared<Scheduler>(id);
      return scheduler;
    }

    ~Scheduler() override {
      this->threads_->clear();
      FEED_WATCHDOG = []() {
      };
    }

    [[nodiscard]] int count(const Pattern &process_pattern = Pattern("#")) const {
      if(this->threads_->empty())
        return 0;
      auto *counter = new atomic_int(0);
      this->threads_->forEach([counter, process_pattern](const Thread_p &thread) {
        if(thread->thread_obj_->vid->matches(process_pattern) && !thread->thread_obj_->get<bool>("halt"))
          counter->fetch_add(1);
      });
      const int c = counter->load();
      delete counter;
      return c;
    }

    void stop() {
      auto map = make_unique<Map<string, int>>();
      auto list = make_unique<List<Thread_p>>();
      this->threads_->forEach([&map,&list](const Thread_p &thread) {
        const string name = thread->thread_obj_->vid->name();
        int count = map->count(name) ? map->at(name) : 0;
        count++;
        if(map->count(name))
          map->erase(name);
        map->insert({name, count});
        list->push_back(thread);
      });
      for(const auto &[name,count]: *map) {
        LOG_WRITE(INFO, this, L("!b%s !y{}!!(s) closing\n", to_string(count), name));
      }
      while(!list->empty()) {
        const Thread_p t = list->back();
        list->pop_back();
        if(!t->thread_obj_->get<bool>("halt"))
          t->halt();
      }
      Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      this->running_ = false;
      LOG_WRITE(INFO, this, L("!yscheduler !b{}!! stopped\n", this->vid->toString()));
    }

    void feed_local_watchdog() {
#ifdef ESP_ARCH
      vTaskDelay(1); // feeds the watchdog for the task
#endif
    }

    void barrier(const string &, const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG_WRITE(INFO, this, L("!mbarrier start: <!y{}!m>!!\n", "main"));
      if(message)
        LOG_WRITE(INFO, this, L("{}\n", message));
      if(!this->router_)
        this->router_ = Router::singleton();

      while(true) {
        this->router_->loop();
        this->feed_local_watchdog();
        std::this_thread::yield();
      }
      LOG_WRITE(INFO, this, L("!mbarrier end: <!g{}!m>!!\n", "main"));
    }

    virtual Thread_p spawn(const Obj_p &thread_obj) {
      if(!thread_obj->vid)
        throw fError("value id required to spawn %s", thread_obj->toString().c_str());
      /* if(this->count(*thread_obj->vid)) {
         LOG_WRITE(ERROR, this, L("!b{} !yprocess!! already running\n",thread_obj->vid->toString()));
         return false;
       }*/
      Thread_p thread = Thread::get_state(thread_obj);
      if(thread->thread_obj_->get<bool>("halt")) {
        throw fError::create(this->toString(), "!b{} !yprocess!! failed to spawn",
                             thread->thread_obj_->vid->toString().c_str());
      } else {
        this->threads_->push_back(thread);
        LOG_WRITE(INFO, this, L("!b{} !ythread!! spawned\n", thread_obj->vid->toString()));
        this->save();
      }
      return thread;
    }

    void save() const override {
      const Lst_p threads = Obj::to_lst();
      this->threads_->forEach([threads](const Thread_p &thread) { threads->lst_add(vri(thread->thread_obj_->vid)); });
      this->rec_set("thread", threads);
      Obj::save();
    }

  public:
    static void *import() {
      if(const Rec_p config = ROUTER_READ(FOS_BOOT_CONFIG_VALUE_ID);
        !config->is_noobj())
        Scheduler::singleton()->rec_set("config", config->rec_get("scheduler")->or_else(noobj()));
      Scheduler::singleton()->save();
      InstBuilder::build(Scheduler::singleton()->vid->add_component("spawn"))
          ->inst_args(rec({{"thread", Obj::to_bcode()}}))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const Thread_p thread = Scheduler::singleton()->spawn(args->arg("thread"));
            return thread->thread_obj_;
          })
          ->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
