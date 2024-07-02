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

#ifndef fhatos_abstract_scheduler_hpp
#define fhatos_abstract_scheduler_hpp

#include <fhatos.hpp>
//
#include <atomic>
#include <structure/furi.hpp>
#include <util/mutex_deque.hpp>
#include FOS_UTIL(mutex.hpp)
#include FOS_PROCESS(process.hpp)
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(thread.hpp)
#include "router/publisher.hpp"
#include "structure/f_bcode.hpp"


namespace fhatos {
  class AbstractScheduler : public IDed, public Publisher, public Mailbox<ptr<Mail>> {
  protected:
    Mutex<> DESTROY_MUTEX;
    MutexDeque<Coroutine *> *COROUTINES = new MutexDeque<Coroutine *>();
    MutexDeque<Fiber *> *FIBERS = new MutexDeque<Fiber *>();
    MutexDeque<Thread *> *THREADS = new MutexDeque<Thread *>();
    MutexDeque<KernelProcess *> *KERNELS = new MutexDeque<KernelProcess *>();
    MutexDeque<ptr<Mail>> inbox;
    Option<ptr<Mail>> pop() override { return this->inbox.pop_front(); }

  public:
    explicit AbstractScheduler(const ID_p &id = share(Router::mintID("127.0.0.1", "kernel/scheduler"))) :
        IDed(id), Publisher(this, this), Mailbox() {}
    ~AbstractScheduler() override {
      delete COROUTINES;
      delete FIBERS;
      delete THREADS;
      delete KERNELS;
    };

    void setup() {
      //////////////// SPAWN
      const fURI threadPattern = this->id()->extend("thread/+");
      this->subscribe(threadPattern, [this, threadPattern](const ptr<Message> &message) {
        const Obj_p obj = message->payload;
        const fURI threadId = message->target.path(this->id()->pathLength() + 1, this->id()->pathLength() + 2);
        if (obj->isNoObj()) {
          LOG(DEBUG,"Destroy thread initiated: %s\n",message->target.toString().c_str());
          this->_destroy(fURI(message->target.lastSegment()));
        } else {
          if (obj->id()->lastSegment() != "thread") {
            LOG(ERROR, "Provided obj must be a /rec/thread: %s\n", OTYPE_STR.at(obj->o_type()));
          } else {
            const auto b = new fBcode<Thread>(threadId, obj);
            if (!this->spawn(b)) {
              LOG_TASK(ERROR, this, "obj type can not be spawned: [stype:%s][utype:%s]\n",
                       OTYPE_STR.at(message->payload->o_type()), message->payload->id()->toString().c_str());
            }
          }

        };
      });
    }


    void shutdown() {
      while (this->next()) {
      }
      THREADS->forEach([this](const Thread *thread) { this->destroy(*thread->id()); });
      this->barrier("shutting_down");
    }

    void barrier(const char *label = "unlabeled", const Supplier<bool> &passPredicate = nullptr) {
      LOG(INFO, "!MScheduler at barrier: <%s>!!\n", label);
      while (this->next()) {
      }
      while (this->next() || (passPredicate && !passPredicate()) || (!passPredicate && this->count() > 0)) {
      }
      LOG(INFO, "!MScheduler completed barrier: <%s>!!\n", label);
    }

    virtual bool spawn(Process *process) { throw fError("%s\n", "Member function spawn() must be implemented"); }
    virtual bool destroy(const ID &processPattern) {
      return this->publish(processPattern, Obj::to_noobj(), TRANSIENT_MESSAGE);
    }

  protected:
    bool next() {
      const Option<ptr<Mail>> mail = this->pop();
      if (!mail.has_value())
        return false;
      mail->get()->first->execute(mail->get()->second);
      return true;
    }
    virtual bool _destroy(const Pattern &processPattern) {
      bool success = DESTROY_MUTEX.lockUnlock<bool>([this, processPattern]() {
        THREADS->remove_if([processPattern, this](Thread *process) {
          if (process->id()->matches(processPattern)) {
            process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id()->toString().c_str(), P_TYPE_STR(process->type));
            // ptr<Thread> temp = ptr<Thread>(process);
            return true;
          }
          return false;
        });
        FIBERS->remove_if([processPattern, this](Fiber *process) {
          if (process->id()->matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id()->toString().c_str(), P_TYPE_STR(process->type));
            // delete process;
            return true;
          }
          return false;
        });
        COROUTINES->remove_if([processPattern, this](Coroutine *process) {
          if (process->id()->matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id()->toString().c_str(), P_TYPE_STR(process->type));
            // delete process;
            return true;
          }
          return false;
        });
        return true;
      });
      if (DEBUG == LOG_TYPE::FOS_LOGGING) {
        LOG(DEBUG, "!b[Current Processes]!!\n");
        LOG(DEBUG, FOS_TAB_2 "!yThreads!!:\n");
        THREADS->forEach([](const Thread *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
        LOG(DEBUG, FOS_TAB_2 "!yFibers!!:\n");
        FIBERS->forEach([](const Fiber *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
        LOG(DEBUG, FOS_TAB_2 "!yCoroutines!!:\n");
        COROUTINES->forEach([](const Coroutine *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id()->toString().c_str()); });
      }
      return success;
    }

  public:
    const List<Process *> *find(const Pattern &processPattern = Pattern("#")) const {
      const auto results = new List<Process *>();
      auto temp = reinterpret_cast<List<Process *> *>(
          THREADS->match([processPattern](const Process *p) { return p->id()->matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          FIBERS->match([processPattern](const Process *p) { return p->id()->matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          COROUTINES->match([processPattern](const Process *p) { return p->id()->matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          KERNELS->match([processPattern](const Process *p) { return p->id()->matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      return results;
    }

    const int count(const Pattern &processPattern = Pattern("#")) const {
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
    }

    bool push(const ptr<Mail> mail) override { return this->inbox.push_back(mail); }
  };
} // namespace fhatos

#endif
