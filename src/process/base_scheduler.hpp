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

#include <fhatos.hpp>
//
#include <atomic>
#include <furi.hpp>
#include <process/process.hpp>
#include <structure/router.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)

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
        LOG_KERNEL_OBJ(INFO, this, "!b{} !y{}!!(s) closing\n", to_string(count).c_str(), name.c_str());
      }
      while(!list->empty()) {
        const Process_p p = list->back();
        list->pop_back();
        if(p->running)
          p->stop();
      }
      router()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      list->clear();
      delete list;
      this->running_ = false;
      LOG_KERNEL_OBJ(INFO, this, "!yscheduler !b{}!! stopped\n", this->vid()->toString().c_str());
    }

    virtual void feed_local_watchdog() = 0;

    [[nodiscard]] bool at_barrier(const string &label) const {
      return this->barrier_.first && this->barrier_.first->name() == label;
    }

    void barrier(const string &name = "unlabeled", const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      this->barrier_ = {id_p(this->vid()->resolve("./barrier/").extend(name)), Obj::to_bcode()};
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier start: <!y{}!m>!!\n", this->barrier_.first->toString().c_str());
      if(message)
        LOG_KERNEL_OBJ(INFO, this, message);
      while(((passPredicate && !passPredicate()) || (!passPredicate && this->running_ && !this->processes_->empty()))
            && (this->barrier_.first && this->barrier_.second)) {
        router()->loop();
        this->feed_local_watchdog();
      }
      LOG_KERNEL_OBJ(INFO, this, "!mbarrier end: <!g{}!m>!!\n", name.c_str());
      this->barrier_.first = nullptr;
      this->barrier_.second = nullptr;
    }

    virtual bool spawn(const Process_p &) = 0;

    void save() override {
      const Lst_p procs = Obj::to_lst();
      this->processes_->forEach([procs](const Process_p &proc) { procs->lst_add(vri(proc->vid())); });
      this->rec_set(vri("process"), procs);
      Obj::save();
    }

  protected:
    static void *base_import(const ptr<BaseScheduler> &scheduler) {
      // ROUTER_WRITE(SCHEDULER_ID, scheduler, RETAIN);
      scheduler
          ///// INSTRUCTIONS
          ->this_add("/:spawn",
                     InstBuilder::build(*INST_FURI)
                     ->type_args(x(0, "thread", ___))
                     ->inst_f([scheduler](const Obj_p &, const InstArgs &args) {
                       const Obj_p &proc = args->arg(0);
                       if(!proc->vid())
                         throw fError("value id required to spawn {}", proc->toString().c_str());
                       if(proc->tid()->has_path("thread"))
                         return dool(scheduler->spawn(make_shared<Thread>(proc)));
                       throw fError("unknown process type: {}\n", proc->tid()->toString().c_str());
                     })
                     // ->doc("spawn a parallel thread of execution")
                     ->create())
          ->this_add("/:stop",
                     InstBuilder::build(*INST_FURI)
                     ->inst_f([scheduler](const Obj_p &, const InstArgs &) {
                       scheduler->stop();
                       return noobj();
                     })
                     ->itype_and_seed(IType::MANY_TO_ZERO)
                     ->create())
          ///// OBJECTS
          ->this_add("/lib/process", Obj::to_rec())
          ->this_add("/lib/thread",
                     Obj::to_rec())
          /*->this_add("::stop",
                     InstBuilder::build(*INST_FURI)
                     ->inst_f([](const Obj_p &, const InstArgs &args) {
                       static_cast<Thread *>(args->arg(0).get())->stop();
                       return _noobj_;
                     })
                     ->itype_and_seed(IType::ONE_TO_ZERO)
                     ->create()))*/
          ->save();
      return nullptr;
    }
  };
}

// namespace fhatos
#endif
