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
#ifndef fhatos_x_scheduler_hpp
#define fhatos_x_scheduler_hpp

#include "../fhatos.hpp"
//
#include <atomic>
#include "../furi.hpp"
#include "process.hpp"
#include "../structure/router.hpp"
#include "../util/mutex_deque.hpp"


namespace fhatos {
  class Sys;
  using Process_p = ptr<Process>;

  class BaseScheduler : public Rec {
  protected:
    MutexDeque<Process_p> *processes_ = new MutexDeque<Process_p>("<scheduler_processes>");
    bool running_ = true;
    Pair<ID_p, BCode_p> barrier_ = {nullptr, nullptr};

  public:
    explicit BaseScheduler(const ID &id = ID("/scheduler")) : Rec(rmap({
                                                                    {"barrier", noobj()},
                                                                    {"process", lst()}}),
                                                                  OType::REC, REC_FURI, id_p(id)) {
      FEED_WATCDOG = [this] {
        this->feed_local_watchdog();
      };
      SCHEDULER_ID = this->vid_;
      LOG_KERNEL_OBJ(INFO, this, "!yscheduler!! started\n");
      // TODO: broadcast when online to trigger bootstrap of other models
      /*Router::singleton()->write(Router::singleton()->vid_, vri(this->vid_), false);
      Router::singleton()->route_subscription(Subscription::create(
        this->vid_,
        p_p(*Router::singleton()->vid_),
        to_inst("on_recv", to_rec({{"msg", to_bcode()}}), [](const Obj_p &lhs, const InstArgs &args) {
          return to_noobj();
        })));*/
    }

    ~BaseScheduler() override {
      delete processes_;
      FEED_WATCDOG = []() {
      };
    }

    [[nodiscard]] int count(const Pattern &process_pattern = Pattern("#")) const {
      if(this->processes_->empty())
        return 0;
      auto *counter = new atomic_int(0);
      this->processes_->forEach([counter, process_pattern](const Process_p &proc) {
        if(proc->vid()->matches(process_pattern) && proc->running)
          counter->fetch_add(1);
      });
      const int c = counter->load();
      delete counter;
      return c;
    }

    void stop() {
      auto map = make_shared<Map<string, int>>();
      auto list = new List<Process_p>();
      this->processes_->forEach([map,list](const Process_p &process) {
        const string name = process->tid()->name();
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
          p->stop();
      }
      Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      list->clear();
      delete list;
      this->running_ = false;
      LOG_KERNEL_OBJ(INFO, this, "!yscheduler !b%s!! stopped\n", this->vid()->toString().c_str());
    }

    virtual void feed_local_watchdog() = 0;

    [[nodiscard]] bool at_barrier(const string &label) const {
      return this->barrier_.first && this->barrier_.first->name() == label;
    }

    void barrier(const string &name = "unlabeled", const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      this->barrier_ = {id_p(this->vid()->resolve("./barrier/").extend(name)), Obj::to_bcode()};
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier start: <!y%s!m>!!\n", this->barrier_.first->toString().c_str());
      if(message)
        LOG_KERNEL_OBJ(INFO, this, message);
      while(((passPredicate && !passPredicate()) || (!passPredicate && this->running_ && !this->processes_->empty()))
            && (this->barrier_.first && this->barrier_.second)) {
        Router::singleton()->loop();
        this->feed_local_watchdog();
      }
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier end: <!g%s!m>!!\n", name.c_str());
      this->barrier_.first = nullptr;
      this->barrier_.second = nullptr;
    }

    virtual bool spawn(const Process_p &) = 0;

    void save() const override {
      const Lst_p procs = Obj::to_lst();
      this->processes_->forEach([procs](const Process_p &proc) { procs->lst_add(vri(proc->vid())); });
      this->rec_set(vri("process"), procs);
      Obj::save();
    }

  protected:
    static void *base_import(const ptr<BaseScheduler> &scheduler) {
      scheduler->this_add("/:spawn",
                          InstBuilder::build(scheduler->vid()->add_component(":spawn"))
                          ->type_args(x(0, "thread", Obj::to_bcode()))
                          ->domain_range(OBJ_FURI, {0, 1}, THREAD_FURI, {1, 1})
                          ->inst_f([scheduler](const Obj_p &, const InstArgs &args) {
                            const auto &p = make_shared<Process>(args->arg(0));
                            p.get()->vid_ = args->arg(0)->vid_;
                            scheduler->spawn(p);
                            return p;
                          })
                          // ->doc("spawn a parallel thread of execution")
                          ->create())
          ->this_add("/:stop",
                     InstBuilder::build(":stop")
                     ->domain_range(OBJ_FURI, {0, 1}, NOOBJ_FURI, {0, 0})
                     ->inst_f([scheduler](const Obj_p &, const InstArgs &) {
                       scheduler->stop();
                       return noobj();
                     })
                     ->create())
          ///// OBJECTS
          ->this_add("/lib/thread", Obj::to_inst(
                       make_shared<InstValue>(make_tuple<InstArgs, InstF_p, Obj_p>(
                         Obj::to_rec({{"loop", Obj::to_bcode()}}),
                         make_shared<std::variant<Obj_p, Cpp_p>>(Obj::to_rec({
                           {":loop", from(vri("loop"), Obj::to_noobj())}})),
                         Obj::to_noobj())),
                       id_p(ID(scheduler->vid_->toString().append("lib/thread")).query({
                         {"dom", OBJ_FURI->toString()},
                         {"dc", "0,1"},
                         {"rng", REC_FURI->toString()},
                         {"rc", "1,1"}
                       }))))
          ->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
