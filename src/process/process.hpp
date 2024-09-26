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
#ifndef fhatos_process_hpp
#define fhatos_process_hpp

#include <atomic>
#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <thread>
#include <util/enums.hpp>
#include <util/ptr_helper.hpp>

namespace fhatos {
  const ID_p THREAD_FURI = share<ID>(ID(REC_FURI->resolve("thread")));
  const ID_p FIBER_FURI = share<ID>(ID(REC_FURI->resolve("fiber")));
  const ID_p COROUTINE_FURI = share<ID>(ID(REC_FURI->resolve("coroutine")));
  CONST_CHAR(ALREADY_STOPPED, "!g[!b%s!g] !y%s!! already stopped\n");
  CONST_CHAR(ALREADY_SETUP, "!g[!b%s!g] !y%s!! already setup\n");

  class Process;
  using Process_p = ptr<Process>;
  static Process_p this_process = nullptr;
  static ptr<thread::id> scheduler_thread = nullptr;

  enum class PType { THREAD, FIBER, COROUTINE };

  static const Enums<PType> ProcessTypes =
      Enums<PType>({{PType::THREAD, "thread"}, {PType::FIBER, "fiber"}, {PType::COROUTINE, "coroutine"}});

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Process : public IDed {
  protected:
    std::atomic_bool running_ = std::atomic_bool(false);

  public:
    const PType ptype;


    explicit Process(const ID &id, const PType pType) : IDed(id_p(id)), ptype(pType) {
    }

    ~Process() override = default;

    virtual void setup() {
      if (this->running_.load()) {
        LOG(WARN, ALREADY_SETUP, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      this->running_.store(true);
      fhatos::this_process = PtrHelper::no_delete(this);
    };

    virtual void loop() {
      if (!this->running_.load()) {
        throw fError("!g[!b%s!g] !y%s!! can't loop when stopped\n", this->id()->toString().c_str(),
                     ProcessTypes.to_chars(this->ptype).c_str());
      }
      fhatos::this_process = PtrHelper::no_delete(this);
    };

    virtual void stop() {
      if (!this->running_.load()) {
        LOG(WARN, ALREADY_STOPPED, this->id()->toString().c_str(), ProcessTypes.to_chars(this->ptype).c_str());
        return;
      }
      fhatos::this_process = PtrHelper::no_delete(this);
      this->running_.store(false);
    };

    bool running() const { return this->running_.load(); }

    virtual void delay(const uint64_t) {
    }; // milliseconds

    virtual void yield() {
    };
  };
} // namespace fhatos

#endif
