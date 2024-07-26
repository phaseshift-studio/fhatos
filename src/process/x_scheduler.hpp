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
#include <process/actor/publisher.hpp>
#include "structure/f_bcode.hpp"

#define LOG_SPAWN(success, process)                                                                                    \
  {                                                                                                                    \
    LOG_TASK((success) ? INFO : ERROR, this, "!b%s!! !y%s!! spawned\n", (process)->id()->toString().c_str(),           \
             P_TYPE_STR((process)->type));                                                                             \
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
    ~XScheduler() override {
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
      LOG_TASK(INFO, this, "!yscheduler!! loaded\n");
    }

    void stop() {
      this->handle_messages();
      for (XProcess *process: *this->find()) {
        if (this->find(*process->id())->at(0)->type == PType::COROUTINE)
          this->_destroy(*process->id());
        else
          this->destroy(*process->id());
        this->handle_messages();
      }
      this->handle_messages();
      this->unsubscribeSource();
      this->handle_messages();
      this->barrier("shutting_down", [this]() {
#ifdef NATIVE
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // delay so _destroy can finish
#endif
        this->handle_messages();
        return true;
      });
    }

    virtual void feedLocalWatchdog() {}

    void barrier(const char *label = "unlabeled", const Supplier<bool> &passPredicate = nullptr) {
      LOG(INFO, "!mScheduler at barrier: <!y%s!m>!!\n", label);
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

    virtual bool spawn(XProcess *) { throw fError::X_REQUIRES_IMPLEMENTATION("Scheduler", "spawn"); }
    virtual bool destroy(const ID &processPattern) {
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
    virtual bool _destroy(const Pattern &processPattern) {
      bool success = RW_PROCESS_MUTEX
                         .write<Bool>([this, processPattern]() {
                           THREADS->remove_if([processPattern, this](Thread *process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_TASK(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          P_TYPE_STR(process->type));
                               } catch (const std::exception &e) {
                                 LOG_EXCEPTION(e);
                               }
                               return true;
                             }
                             return false;
                           });
                           FIBERS->remove_if([processPattern, this](Fiber *process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_TASK(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          P_TYPE_STR(process->type));
                               } catch (const std::exception &e) {
                                 LOG_EXCEPTION(e);
                               }
                               return true;
                             }
                             return false;
                           });
                           COROUTINES->remove_if([processPattern, this](Coroutine *process) {
                             if (process->id()->matches(processPattern)) {
                               try {
                                 if (process->running())
                                   process->stop();
                                 LOG_TASK(INFO, this, "!b%s !y%s!! destroyed\n", process->id()->toString().c_str(),
                                          P_TYPE_STR(process->type));
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
    const List<XProcess *> *find(const Pattern &processPattern = Pattern("#")) {
      return RW_PROCESS_MUTEX.read<List<XProcess *> *>([this, processPattern]() {
        const auto results = new List<XProcess *>();
        auto temp = reinterpret_cast<List<XProcess *> *>(
            THREADS->match([processPattern](const XProcess *p) { return p->id()->matches(processPattern); }));
        for (XProcess *p: *temp) {
          results->push_back(p);
        }
        delete temp;
        temp = reinterpret_cast<List<XProcess *> *>(
            FIBERS->match([processPattern](const XProcess *p) { return p->id()->matches(processPattern); }));
        for (XProcess *p: *temp) {
          results->push_back(p);
        }
        delete temp;
        temp = reinterpret_cast<List<XProcess *> *>(
            COROUTINES->match([processPattern](const XProcess *p) { return p->id()->matches(processPattern); }));
        for (XProcess *p: *temp) {
          results->push_back(p);
        }
        delete temp;
        temp = reinterpret_cast<List<XProcess *> *>(
            KERNELS->match([processPattern](const XProcess *p) { return p->id()->matches(processPattern); }));
        for (XProcess *p: *temp) {
          results->push_back(p);
        }
        delete temp;
        return results;
      });
    }

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