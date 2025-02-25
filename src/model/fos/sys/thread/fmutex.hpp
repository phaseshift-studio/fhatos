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
#ifndef fhatos_fmutex_hpp
#define fhatos_fmutex_hpp
#include "../../../../fhatos.hpp"
#include <any>

namespace fhatos {
  using namespace std;

  // calling lock twice from the same thread will deadlock
  class fMutex final {
  protected:
    std::any handler_;

  public:
    fMutex();

    ~fMutex();

    void lock_shared();

    void unlock_shared();

    void lock();

    void unlock();
  };

  class LockGuard {
    fMutex *mutex_;

  public:
    explicit LockGuard(fMutex &m) : mutex_(&m) {
    }

    ~LockGuard() {
      this->mutex_->unlock();
    }

    LockGuard(const LockGuard &) = delete;
  };
}
#endif
