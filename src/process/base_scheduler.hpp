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
#ifndef fhatos_base_scheduler_hpp
#define fhatos_base_scheduler_hpp

#include "../fhatos.hpp"
//
#include <atomic>
#include "../furi.hpp"
#include "process.hpp"
#include "../structure/router.hpp"
#include "../util/mutex_deque.hpp"
#include STR(ptype/HARDWARE/thread.hpp)


namespace fhatos {
  using Process_p = ptr<Process>;

  class BaseScheduler : public Rec {
  protected:
    MutexDeque<Process_p> *processes_ = new MutexDeque<Process_p>();
    bool running_ = true;
    ptr<Router> router_ = nullptr;

  public:
    explicit BaseScheduler(const ID &id = ID("/scheduler")) :
      Rec(rmap({
              {"barrier", noobj()},
              {"process", lst()}}),
          OType::REC, REC_FURI, id_p(id)) {
      FEED_WATCHDOG = [this] {
        this->feed_local_watchdog();
      };
      SCHEDULER_ID = this->vid;
      LOG_KERNEL_OBJ(INFO, this, "!yscheduler!! started\n");
      // TODO: broadcast when online to trigger bootstrap of other models
      /*Router::singleton()->write(Router::singleton()->vid, vri(this->vid), false);
      Router::singleton()->route_subscription(Subscription::create(
        this->vid,
        p_p(*Router::singleton()->vid),
        to_inst("on_recv", to_rec({{"msg", to_bcode()}}), [](const Obj_p &lhs, const InstArgs &args) {
          return to_noobj();
        })));*/
    }

    ~BaseScheduler() override {
      delete processes_;
      FEED_WATCHDOG = []() {
      };
    }

    [[nodiscard]] int count(const Pattern &process_pattern = Pattern("#")) const {
      if(this->processes_->empty())
        return 0;
      auto *counter = new atomic_int(0);
      this->processes_->forEach([counter, process_pattern](const Process_p &proc) {
        if(proc->vid->matches(process_pattern) && proc->running)
          counter->fetch_add(1);
      });
      const int c = counter->load();
      delete counter;
      return c;
    }

    void stop() {
      auto map = make_unique<Map<string, int>>();
      auto list = make_unique<List<Process_p>>();
      this->processes_->forEach([&map,&list](const Process_p &process) {
        const string name = process->tid->name();
        int count = map->count(name) ? map->at(name) : 0;
        count++;
        if(map->count(name))
          map->erase(name);
        map->insert({name, count});
        list->push_back(process);
      });
      for(const auto &[name,count]: *map) {
        LOG_KERNEL_OBJ(INFO, this, "!b%s !y%s!!(s) closing\n", to_string(count).c_str(), name.c_str());
      }
      while(!list->empty()) {
        const Process_p p = list->back();
        list->pop_back();
        if(p->running)
          p->stop_ = true;
      }
      Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      this->running_ = false;
      LOG_KERNEL_OBJ(INFO, this, "!yscheduler !b%s!! stopped\n", this->vid->toString().c_str());
    }

    virtual void feed_local_watchdog() = 0;

    void barrier(const string &, const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier start: <!y%s!m>!!\n", "main");
      if(message)
        LOG_KERNEL_OBJ(INFO, this, message);
      if(!this->router_)
        this->router_ = Router::singleton();

      while(((passPredicate && !passPredicate()) ||
             (!passPredicate && this->running_ && !this->processes_->empty()))) {
        this->router_->loop();
        this->feed_local_watchdog();
      }
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier end: <!g%s!m>!!\n", "main");
    }

    virtual bool spawn(const Process_p &process) {
      if(!process->vid)
        throw fError("value id required to spawn %s", process->toString().c_str());
      if(this->count(*process->vid)) {
        LOG_KERNEL_OBJ(ERROR, this, "!b%s !yprocess!! already running\n", process->vid->toString().c_str());
        return false;
      }
      process->setup();
      if(!process->running) {
        LOG_KERNEL_OBJ(ERROR, this, "!b%s !yprocess!! failed to spawn\n", process->vid->toString().c_str());
        return false;
      }
      ////////////////////////////////
      if(const Process_p spawned_process = this->raw_spawn(process)) {
        this->processes_->push_back(process);
        Router::singleton()->subscribe(
            Subscription::create(this->vid,
                                 p_p(*spawned_process->vid),
                                 [this,spawned_process](const Obj_p &, const InstArgs &args) {
                                   if(args->arg("payload")->is_noobj()) {
                                     if(spawned_process && spawned_process.get() && !spawned_process->is_noobj())
                                       spawned_process->stop(); // = true;
                                     Router::singleton()->unsubscribe(*this->vid, *spawned_process->vid);
                                   }
                                   return Obj::to_noobj();
                                 }));
        LOG_KERNEL_OBJ(INFO, this, "!b%s !yprocess!! spawned\n", spawned_process->vid->toString().c_str());
        this->save();
      } else {
        throw fError("!b%s!! !yprocess!! failed to spawn", process->vid->toString().c_str());
      }
      return true;
    }

    void save() const override {
      const Lst_p procs = Obj::to_lst();
      this->processes_->forEach([procs](const Process_p &proc) { procs->lst_add(vri(proc->vid)); });
      this->rec_set("process", procs);
      Obj::save();
    }

  protected:
    virtual Process_p raw_spawn(const Process_p &) = 0;

    static void *base_import(const ptr<BaseScheduler> &scheduler) {
      if(const Rec_p config = Router::singleton()->read(FOS_BOOT_CONFIG_VALUE_ID);
        !config->is_noobj())
        scheduler->rec_set("config", config->rec_get("scheduler")->or_else(noobj()));
      InstBuilder::build(scheduler->vid->extend(":spawn"))
          ->type_args(x(0, "obj", Obj::to_bcode()))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_f([scheduler](const Obj_p &, const InstArgs &args) {
            const auto &p = make_shared<Thread>(args->arg(0));
            scheduler->spawn(p);
            return p;
          })
          ->save();
      /* InstBuilder::build(scheduler->vid->extend(":stop"))
           ->inst_args(rec())
           ->domain_range(OBJ_FURI, {0, 1}, NOOBJ_FURI, {0, 0})
           ->inst_f([scheduler](const Obj_p &, const InstArgs &) {
             scheduler->stop();
             return noobj();
           })
           ->save();*/
      ///// OBJECTS
      Router::singleton()->write(SCHEDULER_ID->retract().extend("lib/thread"),
                                 Obj::to_rec({{":loop", Obj::to_bcode()}}));
      scheduler->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
