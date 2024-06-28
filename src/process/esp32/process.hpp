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

#ifndef fhatos_process_hpp
#define fhatos_process_hpp

#include <fhatos.hpp>
#include <structure/furi.hpp>
//

namespace fhatos {
  enum PType { THREAD, FIBER, COROUTINE, KERNEL };

  static const char *P_TYPE_STR(const PType pType) {
    switch (pType) {
      case THREAD:
        return "thread";
      case FIBER:
        return "fiber";
      case COROUTINE:
        return "coroutine";
      case KERNEL:
        return "kernel";
      default:
        return "<unknown process>";
    }
  }

  class Process : public IDed {
  protected:
    bool _running = false;

  public:
    const PType type;

    explicit Process(const ID &id, const PType pType)
      : IDed(share(id)), type(pType) {
    }

    //~Process() { this->stop(); }

    virtual void setup() {
      this->_running = true;
    };

    virtual void loop() {
    }

    virtual void stop() {
      this->_running = false;
    };

    bool running() const { return this->_running; }

    virtual void delay(const uint64_t milliseconds) {
    };

    virtual void yield() {
    };
  };

  class KernelProcess : public Process {
  public:
    explicit KernelProcess(const ID &id) : Process(id, KERNEL) {
    }

    void delay(const uint64_t milliseconds) override { ::delay; }

    void yield() override { ::yield(); }
  };
} // namespace fhatos

#endif
