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
#ifndef fhatos_x_process_hpp
#define fhatos_x_process_hpp

#include <atomic>
#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/enums.hpp>

namespace fhatos {


  enum class PType { THREAD, FIBER, COROUTINE, KERNEL };
  static const Enums<PType> ProcessTypes = Enums<PType>(
      {{PType::THREAD, "thread"}, {PType::FIBER, "fiber"}, {PType::COROUTINE, "coroutine"}, {PType::KERNEL, "kernel"}});

  class XProcess : public IDed {

  protected:
    std::atomic_bool _running = std::atomic_bool(false);

  public:
    const PType type;

    explicit XProcess(const ID &id, const PType pType) : IDed(share(id)), type(pType) {}

    virtual ~XProcess() = default;

    virtual void setup() { this->_running.store(true); };

    virtual void loop() {}

    virtual void stop() { this->_running.store(false); };

    bool running() const { return this->_running.load(); }

    virtual void delay(const uint64_t){}; // milliseconds

    virtual void yield(){}; // throw fError::X_REQUIRES_IMPLEMENTATION("XProcess", "yield"); };
  };

  class XKernel : public XProcess {
  public:
    explicit XKernel(const ID &id) : XProcess(id, PType::KERNEL) {}
  };
} // namespace fhatos

#endif
