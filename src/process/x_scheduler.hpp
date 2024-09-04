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
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(thread.hpp)
#include <language/f_bcode.hpp>
#include <process/actor/publisher.hpp>
#include <structure/pubsub.hpp>
#include <util/mutex_rw.hpp>

#define LOG_SPAWN(success, process)                                                                                    \
 {                                                                                                                     \
 LOG_PROCESS((success) ? INFO : ERROR, this, "!b%s!! !y%s!! %s\n", (process)->id()->toString().c_str(),                \
            ProcessTypes.toChars((process)->ptype).c_str(), (success) ? "spawned" : "!r!_not spawned!!");              \
 }

#define LOG_DESTROY(success, process, scheduler)                                                                       \
{                                                                                                                      \
LOG_PROCESS((success) ? INFO : ERROR, (scheduler), "!b%s!! !y%s!! %s\n", (process)->id()->toString().c_str(),          \
ProcessTypes.toChars((process)->ptype).c_str(), (success) ? "destroyed" : "!r!_not destroyed!!");                      \
}


namespace fhatos {
  using Process_p = ptr<Process>;

  class XScheduler : public IDed, public Mailbox {
  protected:
    ptr<MutexDeque<Process_p>> processes_ = ptr<MutexDeque<Process_p>>(new MutexDeque<Process_p>());
    MutexDeque<Mail_p> inbox_;
    bool running = false;
    ptr<string> current_barrier = nullptr;

  public:
    explicit XScheduler(const ID &id = ID("/scheduler/")): IDed(share(id)), Mailbox() {
    }

    [[nodiscard]] int count(const Pattern &processPattern = Pattern("#")) const {
      if (this->processes_->empty())
        return 0;
      auto *counter = new atomic_int(0);
      this->processes_->forEach([counter,processPattern](const Process_p &proc) {
        if (proc->id()->matches(processPattern) && proc->running())
          counter->fetch_add(1);
      });
      const int c = counter->load();
      delete counter;
      return c;
    }

    static bool isThread(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/thread"); }

    static bool isFiber(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/fiber"); }

    static bool isCoroutine(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/coroutine"); }

    bool recv_mail(const Mail_p &mail) override { return this->inbox_.push_back(mail); }

    virtual void setup() {
      MESSAGE_INTERCEPT = [this](const ID &, const ID &target, const Obj_p &payload, const bool retain) {
        if (!retain || !payload->is_rec())
          return;
        if (isThread(payload)) {
          this->spawn(Process_p(new fBcode<Thread>(target, payload)));
        } else if (isFiber(payload)) {
          this->spawn(Process_p(new fBcode<Fiber>(target, payload)));
        } else if (isCoroutine(payload)) {
          this->spawn(Process_p(new fBcode<Coroutine>(target, payload)));
        }
      };
      this->running = true;
      LOG_PROCESS(INFO, this, "!yscheduler!! loaded\n");
    }

    void stop() {
      auto *threadCount = new atomic_int(0);
      auto *fiberCount = new atomic_int(0);
      auto *coroutineCount = new atomic_int(0);
      this->processes_->forEach([threadCount,fiberCount,coroutineCount](const Process_p &proc) {
        switch (proc->ptype) {
          case PType::THREAD:
            threadCount->fetch_add(1);
            break;
          case PType::FIBER:
            fiberCount->fetch_add(1);
            break;
          case PType::COROUTINE:
            coroutineCount->fetch_add(1);
            break;
        }
      });
      LOG_PROCESS(INFO, this, "!yStopping!g %i !ythreads!! | !g%i !yfibers!! | !g%i !ycoroutines!!\n",
                  threadCount->load(),
                  fiberCount->load(),
                  coroutineCount->load());
      delete threadCount;
      delete fiberCount;
      delete coroutineCount;
      router()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
      List<Process_p> *list = new List<Process_p>();
      this->processes_->forEach([list](const Process_p &proc) {
        list->push_back(proc);
      });
      while (!list->empty()) {
        const Process_p p = list->back();
        list->pop_back();
        if (PType::COROUTINE == p->ptype) {
          LOG_DESTROY(true, p, this);
        }
        if (p->running())
          p->stop();
      }
      this->processes_->clear();
      this->running = false;
      LOG_PROCESS(INFO, this, "!yscheduler !b%s!! stopped\n", this->id()->toString().c_str());
    }

    virtual void feedLocalWatchdog() {
    }

    [[nodiscard]] bool at_barrier(const string &label) const {
      return this->current_barrier && *this->current_barrier == label;
    }

    void barrier(const char *label = "unlabeled", const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG(INFO, "!mScheduler at barrier: <!y%s!m>!!\n", label);
      this->current_barrier = share(string(label));
      if (message)
        LOG(INFO, message);
      /// barrier break with noobj
      /*this->subscribe("", [this, label](const Message_p &message) {
        if (message->payload->is_noobj())
          this->stop();
      });*/
      while (this->read_mail() || (passPredicate && !passPredicate()) || (
               !passPredicate && this->running && !this->processes_->empty())) {
        this->feedLocalWatchdog();
      }
      LOG(INFO, "!mScheduler completed barrier: <!g%s!m>!!\n", label);
      this->current_barrier = nullptr;
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
      mail->get()->first->execute(mail->get()->second);
      return true;
    }
  };
} // namespace fhatos
#endif
