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
#include <process/x_process.hpp>
#include <structure/furi.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(thread.hpp)
#include <language/f_bcode.hpp>
#include <process/actor/publisher.hpp>
#include <util/mutex_rw.hpp>

#define LOG_SPAWN(success, process)                                                                                    \
  {                                                                                                                    \
    LOG_PROCESS((success) ? INFO : ERROR, this, "!b%s!! !y%s!! spawned\n", (process)->id()->toString().c_str(),           \
             ProcessTypes.toChars((process)->type));                                                                   \
  }


namespace fhatos {
  class XScheduler : public IDed, public Publisher, public Mailbox<Mail_p> {
  protected:
    MutexRW<> RW_PROCESS_MUTEX;
    MutexDeque<Coroutine *> *COROUTINES = new MutexDeque<Coroutine *>();
    MutexDeque<Fiber *> *FIBERS = new MutexDeque<Fiber *>();
    MutexDeque<Thread *> *THREADS = new MutexDeque<Thread *>();
    MutexDeque<XKernel *> *KERNELS = new MutexDeque<XKernel *>();
    MutexDeque<Mail_p> inbox;
    Option<Mail_p> pop() override { return this->inbox.pop_front(); }

  public:
    explicit XScheduler(const ID &id = ID("/scheduler/")) : IDed(share(id)), Publisher(this, this), Mailbox() {}
    virtual ~XScheduler() override {
      delete COROUTINES;
      delete FIBERS;
      delete THREADS;
      delete KERNELS;
    };

    static bool isThread(const Obj_p &obj) { return obj->id()->equals("/rec/thread"); }
    static bool isFiber(const Obj_p &obj) { return obj->id()->equals("/rec/fiber"); }
    static bool isCoroutine(const Obj_p &obj) { return obj->id()->equals("/rec/coroutine"); }

    virtual void setup() {
      MESSAGE_INTERCEPT = [this](const ID &, const ID &target, const Obj_p &payload, const bool retain) {
        if (!retain || !payload->isRec())
          return;
        if (isThread(payload)) {
          this->spawn(new fBcode<Thread>(target, payload));
        } else if (isFiber(payload)) {
          this->spawn(new fBcode<Fiber>(target, payload));
        } else if (isCoroutine(payload)) {
          this->spawn(new fBcode<Coroutine>(target, payload));
        }
      };
      LOG_PROCESS(INFO, this, "!yscheduler!! loaded\n");
    }

    void stop() {
      auto *lists = new List<MutexDeque<XProcess *> *>();
      lists->push_back(reinterpret_cast<MutexDeque<XProcess *> *>(COROUTINES));
      lists->push_back(reinterpret_cast<MutexDeque<XProcess *> *>(FIBERS));
      lists->push_back(reinterpret_cast<MutexDeque<XProcess *> *>(THREADS));
      for (const auto &procs: *lists) {
        this->handle_messages();
        procs->forEach([this](const auto &process) {
          this->_kill(*process->id());
          this->handle_messages();
        });
      }
      this->handle_messages();
      this->unsubscribeSource();
      this->handle_messages();
      delete lists;
      this->barrier("shutting_down", [this]() {
#ifdef NATIVE
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // delay so _kill can finish
#endif
        this->handle_messages();
        return true;
      });
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
      while (this->next()) {
        this->feedLocalWatchdog();
      }
      while (this->next() || (passPredicate && !passPredicate()) || (!passPredicate && this->count() > 0)) {
        this->feedLocalWatchdog();
      }
      LOG(INFO, "!mScheduler completed barrier: <!g%s!m>!!\n", label);
    }

    virtual bool spawn(XProcess*) = 0;
    virtual bool kill(const ID &processPattern) {
      return this->publish(processPattern, Obj::to_noobj(), TRANSIENT_MESSAGE);
    }

  protected:
    void handle_messages() {
      while (this->next()) {
      }
    }
    bool next() {
      const Option<ptr<Mail>> mail = this->pop();
      if (!mail.has_value())
        return false;
      mail->get()->first->execute(mail->get()->second);
      return true;
    }
    virtual bool _kill(const Pattern &processPattern) {
      bool success = RW_PROCESS_MUTEX
                         .write<Bool>([this, processPattern]() {
                           // auto &gaslight1 = *reinterpret_cast<MutexDeque<ptr<Thread>> *>(THREADS);
                           THREADS->remove_if([processPattern, this](const auto &process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_PROCESS(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          ProcessTypes.toChars(process->type));
                               } catch (const std::exception &e) {
                                 LOG_EXCEPTION(e);
                               }
                               return true;
                             }
                             return false;
                           });
                           // auto &gaslight2 = *reinterpret_cast<MutexDeque<ptr<Fiber>> *>(FIBERS);
                           FIBERS->remove_if([processPattern, this](const auto &process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_PROCESS(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          ProcessTypes.toChars(process->type));
                               } catch (const std::exception &e) {
                                 LOG_EXCEPTION(e);
                               }
                               return true;
                             }
                             return false;
                           });
                           // auto &gaslight3 = *reinterpret_cast<MutexDeque<ptr<Coroutine>> *>(COROUTINES);
                           COROUTINES->remove_if([processPattern, this](const auto &process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_PROCESS(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          ProcessTypes.toChars(process->type));
                               } catch (const std::exception &e) {
                                 LOG_EXCEPTION(e);
                               }
                               return true;
                             }
                             return false;
                           });
                           return share(Bool(true));
                         })
                         ->bool_value();
      LOG(DEBUG, "!b[Current Processes]!!\n");
      LOG(DEBUG, FOS_TAB_2 "!yThreads!!:\n");
      THREADS->forEach([](const Thread *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
      LOG(DEBUG, FOS_TAB_2 "!yFibers!!:\n");
      FIBERS->forEach([](const Fiber *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
      LOG(DEBUG, FOS_TAB_2 "!yCoroutines!!:\n");
      COROUTINES->forEach([](const Coroutine *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
      return success;
    }

  public:
    int count(const Pattern &processPattern = Pattern("#")) {
      return RW_PROCESS_MUTEX.read<int>([this, processPattern]() {
        if (processPattern.equals(Pattern("#")))
          return THREADS->size() + FIBERS->size() + COROUTINES->size() /*+ KERNELS->size()*/;
        auto *counter = new std::atomic(0);
        THREADS->forEach([counter, processPattern](const Thread *process) {
          if (process->id()->matches(processPattern))
            counter->fetch_add(1);
        });
        FIBERS->forEach([counter, processPattern](const Fiber *process) {
          if (process->id()->matches(processPattern))
            counter->fetch_add(1);
        });

        COROUTINES->forEach([counter, processPattern](const Coroutine *process) {
          if (process->id()->matches(processPattern))
            counter->fetch_add(1);
        });
        /* KERNELS->forEach([counter, processPattern](const KernelProcess *process) {
           if (process->id()->matches(processPattern))
             counter->fetch_add(1);
         });*/
        const int temp = counter->load();
        delete counter;
        return temp;
      });
    }

    bool push(const ptr<Mail> mail) override { return this->inbox.push_back(mail); }
  };
} // namespace fhatos

#endif
