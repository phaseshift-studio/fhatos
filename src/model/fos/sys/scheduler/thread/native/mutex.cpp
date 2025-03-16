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
#ifdef NATIVE
#include "../mutex.hpp"
#include <shared_mutex>
namespace fhatos {
  using namespace std;
    // calling lock twice from the same thread will deadlock
    Mutex::Mutex() : handler_(new std::shared_mutex()) {
    }

    Mutex::~Mutex() {
	    const auto m = std::any_cast<std::shared_mutex*>(this->handler_);
      m->unlock();
      delete m;
	  }

    void Mutex::lock_shared() {
      std::any_cast< std::shared_mutex*>(this->handler_)->lock_shared();
    }
    void Mutex::unlock_shared() {
      std::any_cast< std::shared_mutex*>(this->handler_)->unlock_shared();
    }
   void Mutex::lock() {
      std::any_cast< std::shared_mutex*>(this->handler_)->lock();
    }
    void Mutex::unlock()  {
      std::any_cast<std::shared_mutex*>(this->handler_)->unlock();
    }
}
#endif
