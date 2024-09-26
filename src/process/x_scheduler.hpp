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
#include <util/mutex_deque.hpp>
#include <structure/router.hpp>
#include <structure/pubsub.hpp>
#include <util/mutex_rw.hpp>

#define LOG_SPAWN(success, process)                                                                                    \
  {                                                                                                                    \
    LOG_SCHEDULER((success) ? INFO : ERROR, "!b%s!! !y%s!! %s\n",                                                      \
                ProcessTypes.to_chars((process)->ptype).c_str(),                                                       \
                (success) ? "spawned" : "!r!_not spawned!!");                                                          \
  }

#define LOG_DESTROY(success, process, scheduler)                                                                       \
  {                                                                                                                    \
    LOG_PROCESS((success) ? INFO : ERROR, (scheduler), "!b%s!! !y%s!! %s\n", (process)->id()->toString().c_str(),      \
                ProcessTypes.to_chars((process)->ptype).c_str(), (success) ? "destroyed" : "!r!_not destroyed!!");     \
  }


namespace fhatos {
  using Process_p = ptr<Process>;

  class XScheduler : public IDed, public Mailbox {
    friend class System;

  protected:
    ptr<MutexDeque<Process_p>> processes_ = std::make_shared<MutexDeque<Process_p>>();
    MutexDeque<Mail_p> inbox_;
    bool running_ = false;
    ID_p current_barrier_ = nullptr;

  public:
    explicit XScheduler(const ID &id = ID("/scheduler/")) : IDed(share(id)), Mailbox() {
    }

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
      SCHEDULER_INTERCEPT = [this](const ID &target, const Obj_p &payload, const bool retain) -> bool {
        if (retain && payload->is_rec() && (
              payload->id()->matches(THREAD_FURI->extend("#")) ||
              payload->id()->matches(FIBER_FURI->extend("#")) ||
              payload->id()->matches(COROUTINE_FURI->extend("#")))) {
          LOG_SCHEDULER(DEBUG, "intercepting retained !yprocess!! %s\n", payload->toString().c_str());
          PROCESS_SPAWNER(*payload->id(), target);
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
      this->processes_->forEach([list,thread_count, fiber_count, coroutine_count](const Process_p &proc) {
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
      LOG_SCHEDULER(INFO, "!yStopping!g %i !ythreads!! | !g%i !yfibers!! | !g%i !ycoroutines!!\n",
                    thread_count->load(), fiber_count->load(), coroutine_count->load());
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

    virtual void feed_local_watchdog() {
    }

    [[nodiscard]] bool at_barrier(const string &label) const {
      return this->current_barrier_ && *this->current_barrier_ == label;
    }

    void barrier(const ID &label = ID("unlabeled"), const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG_SCHEDULER(INFO, "!mbarrier start: <!y%s!m>!!\n", label.toString().c_str());
      this->current_barrier_ = id_p(label);

      if (message)
        LOG_SCHEDULER(INFO, message);
      /// barrier break with noobj
      /*this->subscribe("", [this, label](const Message_p &message) {
        if (message->payload->is_noobj())
          this->stop();
      });*/
      while (this->read_mail() || (passPredicate && !passPredicate()) ||
             (!passPredicate && this->running_ && !this->processes_->empty())) {
        router()->loop();
        this->feed_local_watchdog();
      }
      LOG_SCHEDULER(INFO, "!mbarrier end: <!g%s!m>!!\n", label.toString().c_str());
      this->current_barrier_ = nullptr;
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
