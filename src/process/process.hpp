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
#include <util/enums.hpp>
#include "furi.hpp"

namespace fhatos {

  enum class PType { THREAD, FIBER, COROUTINE };
  static const Enums<PType> ProcessTypes = Enums<PType>(
      {{PType::THREAD, "thread"}, {PType::FIBER, "fiber"}, {PType::COROUTINE, "coroutine"}});

  class Process : public IDed {

  protected:
    std::atomic_bool _running = std::atomic_bool(false);

  public:
    const PType ptype;

    explicit Process(const ID &id, const PType pType) : IDed(id_p(id)), ptype(pType) {}

    ~Process() override = default;

    virtual void setup() {
      if (this->_running.load()) {
        LOG(WARN, "!g[!b%s!g] !y%s!! already setup\n", this->id()->toString().c_str(), ProcessTypes.toChars(this->ptype));
        return;
      }
      this->_running.store(true);
    };

    virtual void loop() {
      if (!this->_running.load()) {
        throw fError("!g[!b%s!g] !y%s!! can't loop when stopped\n", this->id()->toString().c_str(),
                     ProcessTypes.toChars(this->ptype));
      }
    };

    virtual void stop() {
      if (!this->_running.load()) {
        LOG(WARN, "!g[!b%s!g] !y%s!! already stopped\n", this->id()->toString().c_str(), ProcessTypes.toChars(this->ptype));
        return;
      }
      this->_running.store(false);
    };

    bool running() const { return this->_running.load(); }

    virtual void delay(const uint64_t) {}; // milliseconds

    virtual void yield() {};
  };
} // namespace fhatos

#endif
