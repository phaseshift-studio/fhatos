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
  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class AbstractScheduler : public IDed, public Publisher<ROUTER>, public Mailbox<ptr<Mail>> {

  protected:
    Mutex<> DESTROY_MUTEX;
    MutexDeque<Coroutine *> *COROUTINES = new MutexDeque<Coroutine *>();
    MutexDeque<Fiber *> *FIBERS = new MutexDeque<Fiber *>();
    MutexDeque<Thread *> *THREADS = new MutexDeque<Thread *>();
    MutexDeque<KernelProcess *> *KERNELS = new MutexDeque<KernelProcess *>();
    MutexDeque<ptr<Mail>> inbox;
    Option<ptr<Mail>> pop() override { return this->inbox.pop_front(); }

  public:
    explicit AbstractScheduler(const ID &id = ROUTER::mintID("scheduler", "kernel")) :
        IDed(id), Publisher<ROUTER>(this, this), Mailbox() {
      this->subscribe(id.query("?spawn"), [this](const Message &message) {
        const Rec rec = message.payload->toRec();
        const auto b = new fBcode<Thread,ROUTER>(rec.get<Uri>(new Str("id"))->value(), new Rec(*rec.value()));
        this->spawn(b);
      });
      this->subscribe(id.query("?destroy"), [this](const Message &message) {
        const Uri uri = message.payload->toUri();
        LOG_TASK(DEBUG, this, "received ?destroy=%s from %s\n", uri.toString().c_str(),
                 message.source.toString().c_str());
        this->_destroy(uri.value());
      });
    }
    ~AbstractScheduler() override {
      delete COROUTINES;
      delete FIBERS;
      delete THREADS;
      delete KERNELS;
    };

    void shutdown() {
      while (this->next()) {
      }
      THREADS->forEach([this](const Thread *thread) { this->destroy(thread->id()); });
      this->barrier("shutting_down");
    }

    void barrier(const char *label = "unlabled", const Supplier<bool> &passPredicate = nullptr) {
      LOG(INFO, "!MScheduler at barrier: <%s>!!\n", label);
      while (this->next()) {
      }
      while (this->next() || (passPredicate && !passPredicate()) || (!passPredicate && THREADS->size(false) > 0)) {
      }
      LOG(INFO, "!MScheduler completed barrier: <%s>!!\n", label);
    }

    virtual bool spawn(Process *process) { throw fError("Member function spawn() must be implemented"); }
    virtual bool destroy(const ID &processPattern) {
      return this->publish(this->id().query("?destroy"), BinaryObj<>::fromObj(new Uri(processPattern)),
                           TRANSIENT_MESSAGE);
    }

  protected:
    bool next() {
      const Option<ptr<Mail>> mail = this->pop();
      if (!mail.has_value())
        return false;
      mail->get()->first->execute(*mail->get()->second);
      /// delete mail->second.payload;
      return true;
    }
    virtual bool _destroy(const Pattern &processPattern) {
      bool success = DESTROY_MUTEX.lockUnlock<bool>([this, processPattern]() {
        THREADS->remove_if([processPattern, this](Thread *process) {
          if (process->id().matches(processPattern)) {
            process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        FIBERS->remove_if([processPattern, this](Fiber *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        COROUTINES->remove_if([processPattern, this](Coroutine *process) {
          if (process->id().matches(processPattern)) {
            if (process->running())
              process->stop();
            LOG_TASK(INFO, this, "!m%s!! %s destroyed\n", process->id().toString().c_str(), P_TYPE_STR(process->type));
            delete process;
            return true;
          }
          return false;
        });
        return true;
      });
      if (DEBUG == _logging) {
        LOG(DEBUG, "!b[Current Processes]!!\n");
        LOG(DEBUG, FOS_TAB_2 "!yThreads!!:\n");
        THREADS->forEach([](const Thread *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id().toString().c_str()); });
        LOG(DEBUG, FOS_TAB_2 "!yFibers!!:\n");
        FIBERS->forEach([](const Fiber *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id().toString().c_str()); });
        LOG(DEBUG, FOS_TAB_2 "!yCoroutines!!:\n");
        COROUTINES->forEach([](const Coroutine *p) { LOG(DEBUG, FOS_TAB_3 "!m%s!!\n", p->id().toString().c_str()); });
      }
      return success;
    }

  public:
    const List<Process *> *find(const Pattern &processPattern = Pattern("#")) const {
      const auto results = new List<Process *>();
      auto temp = reinterpret_cast<List<Process *> *>(
          THREADS->match([processPattern](const Process *p) { return p->id().matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          FIBERS->match([processPattern](const Process *p) { return p->id().matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          COROUTINES->match([processPattern](const Process *p) { return p->id().matches(processPattern); }));
      for (Process *p: *temp) {
        results->push_back(p);
      }
      delete temp;
      temp = reinterpret_cast<List<Process *> *>(
          KERNELS->match([processPattern](const Process *p) { return p->id().matches(processPattern); }));
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
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      FIBERS->forEach([counter, processPattern](const Fiber *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });

      COROUTINES->forEach([counter, processPattern](const Coroutine *process) {
        if (process->id().matches(processPattern))
          counter->fetch_add(1);
      });
      /* KERNELS->forEach([counter, processPattern](const KernelProcess *process) {
         if (process->id().matches(processPattern))
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
