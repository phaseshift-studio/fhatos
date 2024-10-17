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

#ifndef fhatos_std_mutex_hpp
#define fhatos_std_mutex_hpp

#include <chrono>
#include <ctime>
#include <mutex>
#include <sys/time.h>

namespace fhatos {
  template<uint16_t WAIT_TIME_MS = 250>
  class Mutex {
    std::mutex xmutex = std::mutex();
    string _label;

  public:
    explicit Mutex(const char *label = "<anon>") : _label(string(label)) {}

    template<typename T = void *>
    T lockUnlock(const std::function<T()> criticalFunction, const uint16_t millisecondsWait = WAIT_TIME_MS) {
      try {
        if (this->lock(millisecondsWait)) {
          T t = criticalFunction();
          this->unlock();
          return t;
        } else {
          throw fError("Unable to lock mutex %s", this->_label.c_str());
        }
      } catch (const std::exception &) {
        this->xmutex.unlock();
        throw;
      }
    }

    bool lock(const uint16_t millisecondsWait = WAIT_TIME_MS) {
      const long timestamp =
              std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                      .count();
      while (true) {
        if (this->xmutex.try_lock()) {
          return true;
        } else if ((std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                            .count() -
                    timestamp) > millisecondsWait) {
          return false;
        }
      }
    }

    bool unlock() {
      this->xmutex.unlock();
      return true;
    }
  };
} // namespace fhatos

#endif
