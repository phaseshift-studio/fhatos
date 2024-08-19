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
  {                                                                                                                    \
    LOG_PROCESS((success) ? INFO : ERROR, this, "!b%s!! !y%s!! %s\n", (process)->id()->toString().c_str(),             \
                ProcessTypes.toChars((process)->ptype), (success) ? "spawned" : "!r!_spawned!!");                      \
  }


namespace fhatos {
  using Process_p = ptr<Process>;
  class XScheduler : public IDed, public Mailbox {
  protected:
    MutexRW<> processes_mutex_ = MutexRW("<scheduler processes mutex>");
    ptr<Map<const ID_p, Process_p, furi_p_less>> processes_ = share(Map<const ID_p, Process_p, furi_p_less>());
    MutexDeque<Mail_p> inbox_;
    bool running = false;

    // bool isInMainThread() { return this_thread::get_id() == this->main_thread; }


  public:
    explicit XScheduler(const ID &id = ID("/scheduler/")) : IDed(share(id)), Mailbox() {}

    int count(const Pattern &processPattern = Pattern("#")) {
      if (this->processes_->empty())
        return 0;
      return this->processes_mutex_.read<int>([this, processPattern]() {
        int counter = 0;
        for (const auto &[id, proc]: *this->processes_) {
          if (id->matches(processPattern) && proc->running())
            counter++;
        }
        return counter;
      });
    }


    static bool isThread(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/thread"); }
    static bool isFiber(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/fiber"); }
    static bool isCoroutine(const Obj_p &obj) { return obj->id()->equals(FOS_TYPE_PREFIX "rec/coroutine"); }
    bool recv_mail(const Mail_p &mail) override { return this->inbox_.push_back(mail); }
    virtual void setup() {
      MESSAGE_INTERCEPT = [this](const ID &, const ID &target, const Obj_p &payload, const bool retain) {
        if (!retain || !payload->isRec())
          return;
        if (isThread(payload)) {
          this->spawn(ptr<Process>(new fBcode<Thread>(target, payload)));
        } else if (isFiber(payload)) {
          this->spawn(ptr<Process>(new fBcode<Fiber>(target, payload)));
        } else if (isCoroutine(payload)) {
          this->spawn(ptr<Process>(new fBcode<Coroutine>(target, payload)));
        }
      };
      this->running = true;
      LOG_PROCESS(INFO, this, "!yscheduler!! loaded\n");
    }

    void stop() {
      // if (!this->isInMainThread()) {
      // TODO: console. :shutdown calls stop();
      // return;
      //}
      this->processes_mutex_.read<void *>([this]() {
        int threadCount = 0;
        int fiberCount = 0;
        int coroutineCount = 0;
        for (const auto &[id, proc]: *this->processes_) {
          switch (proc->ptype) {
            case PType::THREAD:
              ++threadCount;
              break;
            case PType::FIBER:
              ++fiberCount;
              break;
            case PType::COROUTINE:
              ++coroutineCount;
              break;
          }
        }
        LOG_PROCESS(INFO, this, "Stopping %i threads | %i fibers | %i coroutines\n", threadCount, fiberCount,
                    coroutineCount);
        return nullptr;
      });
      this->processes_mutex_.read<void *>([this]() {
        for (const auto &[id, proc]: *this->processes_) {
          if (proc->running())
            proc->stop();
          if (proc->ptype == PType::COROUTINE)
            this->kill(*proc->id());
        }
        return nullptr;
      });
      router()->stop();
      this->running = false;
    }

    virtual void feedLocalWatchdog() {}

    void barrier(const char *label = "unlabeled", const Supplier<bool> &passPredicate = nullptr,
                 const char *message = nullptr) {
      LOG(INFO, "!mScheduler at barrier: <!y%s!m>!!\n", label);
      if (message)
        LOG(INFO, message);
      /// barrier break with noobj
      /*this->subscribe("", [this, label](const Message_p &message) {
        if (message->payload->isNoObj())
          this->stop();
      });*/
      while (this->read_mail() || (passPredicate && !passPredicate()) || (!passPredicate && this->count() > 0)) {
        this->feedLocalWatchdog();
      }
      LOG(INFO, "!mScheduler completed barrier: <!g%s!m>!!\n", label);
    }

    virtual bool spawn(const Process_p &) = 0;
    virtual bool kill(const ID &processPattern) {
      return this->inbox_.push_back(share(
          Mail{share(Subscription{.source = *this->id(),
                                  .pattern = processPattern,
                                  .onRecv = [this](const Message_p &message) { this->_kill(message->target); }}),
               share(Message{
                   .source = *this->id(), .target = processPattern, .payload = noobj(), .retain = RETAIN_MESSAGE})}));
    }

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
    ///////////////////////////////////////////////////////////////////////////
    virtual bool _kill(const Pattern &processPattern) {
      return bool(*processes_mutex_.write<bool>([this, processPattern]() {
        const uint8_t size = this->processes_->size();
        List<ID_p> toRemove;
        for (const auto &pair: *this->processes_) {
          if (pair.first->matches(processPattern)) {
            router()->detach(((Structure *) pair.second.get())->pattern());
            toRemove.push_back(pair.first);
          }
        }
        for (const auto &i: toRemove) {
          this->processes_->erase(i);
        }
        return share(this->processes_->size() < size);
      }));
    }
  };
} // namespace fhatos
#endif
