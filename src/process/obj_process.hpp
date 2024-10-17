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
#ifndef fhatos_obj_process_hpp
#define fhatos_obj_process_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/processor/processor.hpp>
#include <structure/router.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS>
  class ProcessObj : public PROCESS {
  protected:
    const Rec_p process_rec_;
    const Uri_p id_uri_;

  public:
    explicit ProcessObj(const ID &id) : PROCESS(id), process_rec_(router()->read(make_shared<ID>(id))->clone()),
                                        id_uri_(Obj::to_uri(id)) {
    }

    void setup() override {
      try {
        PROCESS::setup();
        LOG_PROCESS(DEBUG, this, "Executing setup()-bcode: %s\n",
                    this->process_rec_->rec_get(vri(this->id()->resolve(":setup")))->toString().c_str());
        process(this->process_rec_->rec_get(vri(this->id()->resolve(":setup"))), this->id_uri_);
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void loop() override {
      try {
        if (this->running_.load()) {
          const BCode_p loop_bcode = this->process_rec_->rec_get(vri(
            this->id()->resolve(":loop"))); // router()->read(this->id())->rec_get(vri(this->id()->resolve(":loop")));
          process(loop_bcode, this->id_uri_);
        }
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void stop() override {
      try {
        if (this->running_.load()) {
          LOG_PROCESS(DEBUG, this, "Executing stop()-bcode: %s\n",
                      this->process_rec_->rec_get(vri(this->id()->resolve(":stop")))->toString().c_str());
          process(this->process_rec_->rec_get(vri(this->id()->resolve(":stop"))), this->id_uri_);
          PROCESS::stop();
        }
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
      }
    }
  };

  class ThreadObj : public ProcessObj<Thread> {
  public:
    explicit ThreadObj(const ID &id) : ProcessObj(id) {
    }
  };

  class FiberObj : public ProcessObj<Fiber> {
  public:
    explicit FiberObj(const ID &id) : ProcessObj(id) {
    }
  };

  class CoroutineObj : public ProcessObj<Coroutine> {
  public:
    explicit CoroutineObj(const ID &id) : ProcessObj(id) {
    }
  };

  inline void load_process_spawner() {
    PROCESS_SPAWNER = [](const ID &type, const ID &process_id) {
      if (THREAD_FURI->equals(type))
        scheduler()->spawn(std::make_shared<ThreadObj>(process_id));
      else if (FIBER_FURI->equals(type))
        scheduler()->spawn(std::make_shared<FiberObj>(process_id));
      else if (COROUTINE_FURI->equals(type))
        scheduler()->spawn(std::make_shared<CoroutineObj>(process_id));
      else
        throw fError("Unknown process type: %s", type.toString().c_str());
    };
  }

  inline Rec_p load_process(const Process_p &process,
                            const string &filename = __FILE__,
                            const int setup_linenumber = __LINE__,
                            const int loop_linenumber = __LINE__,
                            const int stop_linenumber = __LINE__) {
    const Rec_p record = rec();//Obj::to_rec(make_shared<Obj::RecMap<>>(), id_p(*process->id()));
    record->rec_set(vri(process->id()->extend(":setup")), Insts::to_bcode(
                      [process](const Obj_p &) {
                        process->setup();
                        return noobj();
                      },
                      ID(filename).resolve(string(":") + to_string(setup_linenumber))));
    if (PType::COROUTINE != process->ptype) {
      record->rec_set(vri(process->id()->extend(":loop")),
                      Insts::to_bcode(
                        [process](const Obj_p &) {
                          process->loop();
                          return noobj();
                        },
                        ID(filename).resolve(string(":") + to_string(loop_linenumber))));
    }
    record->rec_set(vri(process->id()->extend(":stop")), Insts::to_bcode(
                      [process](const Obj_p &) {
                        process->setup();
                        return noobj();
                      },
                      ID(filename).resolve(string(":") + to_string(stop_linenumber))));
    return record;
  }
} // namespace fhatos
#endif
