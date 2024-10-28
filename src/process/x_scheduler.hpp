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
#include <structure/pubsub.hpp>
#include <structure/router.hpp>
#include <util/mutex_deque.hpp>

#define LOG_SPAWN(success, process)                                                                                    \
  {                                                                                                                    \
    LOG_SCHEDULER((success) ? INFO : ERROR, "!b%s!! !y%s!! %s\n", ProcessTypes.to_chars((process)->ptype).c_str(),     \
                  (success) ? "spawned" : "!r!_not spawned!!");                                                        \
  }

#define LOG_DESTROY(success, process, scheduler)                                                                       \
  {                                                                                                                    \
    LOG_PROCESS((success) ? INFO : ERROR, (scheduler), "!b%s!! !y%s!! %s\n", (process)->id()->toString().c_str(),      \
                ProcessTypes.to_chars((process)->ptype).c_str(), (success) ? "destroyed" : "!r!_not destroyed!!");     \
  }


namespace fhatos {
  using Process_p = ptr<Process>;

  class XScheduler : public IDed, public Mailbox {
  protected:
    MutexDeque<Process_p> *processes_ = new MutexDeque<Process_p>("<xscheduler_processes>");
    MutexDeque<Mail_p> inbox_ = MutexDeque<Mail_p>("<xscheduler_mail>");
    bool running_ = false;
    Pair<ID_p, BCode_p> barrier_ = {nullptr, nullptr};

  public:
    explicit XScheduler(const ID &id = ID("/scheduler/")) : IDed(share(id)), Mailbox() {}

    ~XScheduler() override { delete processes_; }

    [[nodiscard]] int count(const Pattern &process_pattern = Pattern("#")) const {
      if (this->processes_->empty())
        return 0;
      auto *counter = new atomic_int(0);
      this->processes_->forEach([counter, process_pattern](const Process_p &proc) {
        if (proc->id()->matches(process_pattern) && proc->running())
          counter->fetch_add(1);
      });
      const int c = counter->load();
      delete counter;
      return c;
    }

    bool recv_mail(const Mail_p &mail) override { return this->inbox_.push_back(mail); }

    virtual void setup() {
      SCHEDULER_READ_INTERCEPT = [this](const fURI &furi) -> Objs_p {
        bool proc_branch = this->id()->resolve("./process/").bimatches(furi);
        bool barr_branch =
            this->barrier_.first && this->barrier_.second && this->id()->resolve("./barrier/").bimatches(furi);
        if (proc_branch || barr_branch) {
          Rec_p rec = Obj::to_rec();
          if (proc_branch) {
            auto uris = make_shared<List<Uri_p>>();
            this->processes_->forEach([uris](const Process_p &process) { uris->push_back(vri(process->id())); });
            rec = ObjHelper::encode_lst(this->id()->resolve("./process/"), *uris);
          }
          if (barr_branch) {
            rec->rec_set(vri(this->barrier_.first), this->barrier_.second);
          }
          return rec;
        }
        const bool proc_node = this->id()->resolve("./process/+").bimatches(furi);
        const bool barr_node =
            this->barrier_.first && this->barrier_.second && this->barrier_.first->bimatches(furi);
        if (proc_node || barr_node) {
          const Objs_p objs = Obj::to_objs();
          if (proc_node) {
            if (StringHelper::is_integer(furi.name()))
              return vri(this->processes_->get(stoi(furi.name())).value()->id());
            if (furi.name() == "+" || furi.name() == "#") {
              this->processes_->forEach([objs](const Process_p &process) { objs->add_obj(vri(process->id())); });
            }
          }
          if (barr_node) {
            objs->add_obj(this->barrier_.second);
          }
          return objs;
        }
        return noobj();
      };

      SCHEDULER_WRITE_INTERCEPT = [this](const ID &target, const Obj_p &payload, const bool retain) -> bool {
        if (!retain)
          return false;
        if (payload->is_noobj()) {
          if (this->id()->equals(target)) {
            this->stop();
            return true;
          }
          if (this->barrier_.first && this->barrier_.first->equals(target)) {
            this->barrier_.first = nullptr;
            this->barrier_.second = nullptr;
            return true;
          }
          const Option<Process_p> found_process =
              this->processes_->find([target](const Process_p &process) { return process->id()->equals(target); });
          if (found_process.has_value()) {
            router()->route_unsubscribe(id_p(target));
            found_process.value()->stop();
            return true;
          }
        }
        if (payload->is_rec() &&
            (payload->type()->matches(THREAD_FURI->extend("#")) || payload->type()->matches(FIBER_FURI->extend("#")) ||
             payload->type()->matches(COROUTINE_FURI->extend("#")))) {
          LOG_SCHEDULER(DEBUG, "intercepting retained !yprocess!! %s\n", payload->toString().c_str());
          PROCESS_SPAWNER(ID(*payload->type()), target);
          return true;
        }
        return false;
      };
      this->running_ = true;
      LOG_SCHEDULER(INFO, "!yscheduler!! started\n");
    }

    void stop() {
      auto *thread_count = new atomic_int(0);
      auto *fiber_count = new atomic_int(0);
      auto *coroutine_count = new atomic_int(0);
      auto list = new List<Process_p>();
      this->processes_->forEach([list, thread_count, fiber_count, coroutine_count](const Process_p &proc) {
        switch (proc->ptype) {
          case PType::THREAD:
            thread_count->fetch_add(1);
            break;
          case PType::FIBER:
            fiber_count->fetch_add(1);
            break;
          case PType::COROUTINE:
            coroutine_count->fetch_add(1);
            break;
        }
        list->push_back(proc);
      });
      LOG_SCHEDULER(INFO, "!yStopping!g %i !ythreads!! | !g%i !yfibers!! | !g%i !ycoroutines!!\n", thread_count->load(),
                    fiber_count->load(), coroutine_count->load());
      delete thread_count;
      delete fiber_count;
      delete coroutine_count;
      router()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      while (!list->empty()) {
        const Process_p p = list->back();
        list->pop_back();
        if (PType::COROUTINE == p->ptype) {
          LOG_DESTROY(true, p, this);
        }
        if (p->running())
          p->stop();
      }
      list->clear();
      delete list;
      this->processes_->clear();
      this->running_ = false;
      LOG_SCHEDULER(INFO, "!yscheduler !b%s!! stopped\n", this->id()->toString().c_str());
    }

    virtual void feed_local_watchdog() = 0;

    [[nodiscard]] bool at_barrier(const string &label) const {
      return this->barrier_.first && this->barrier_.first->name() == label;
    }

    void barrier(const string &name = "unlabeled", const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      this->barrier_ = {id_p(this->id()->resolve("./barrier/").extend(name)), Obj::to_bcode()};
      LOG_SCHEDULER(INFO, "!mbarrier start: <!y%s!m>!!\n", this->barrier_.first->toString().c_str());
      if (message)
        LOG_SCHEDULER(INFO, message);
      /// barrier break with noobj
      /*this->subscribe("", [this, label](const Message_p &message) {
        if (message->payload->is_noobj())
          this->stop();
      });*/
      while ((this->read_mail() || (passPredicate && !passPredicate()) ||
              (!passPredicate && this->running_ && !this->processes_->empty())) &&
             (this->barrier_.first && this->barrier_.second)) {
        router()->loop();
        this->feed_local_watchdog();
      }
      LOG_SCHEDULER(INFO, "!mbarrier end: <!g%s!m>!!\n", name.c_str());
      this->barrier_.first = nullptr;
      this->barrier_.second = nullptr;
    }

    virtual bool spawn(const Process_p &) = 0;

  protected:
    bool read_mail() {
      //   if (this->main_thread != this_thread::get_id())
      //     throw fError("Mail can only be read by the primary thread: %i != %i\n", this->main_thread,
      //                this_thread::get_id());
      const Option<ptr<Mail>> mail = this->inbox_.pop_front();
      if (!mail.has_value())
        return false;
      mail->get()->first->on_recv->apply(mail->get()->second->to_rec());
      return true;
    }
  };
} // namespace fhatos
#endif
