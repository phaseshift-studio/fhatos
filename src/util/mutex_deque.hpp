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
#ifndef fhatos_mutex_deque_hpp
#define fhatos_mutex_deque_hpp

#include <fhatos.hpp>
////
#include FOS_MUTEX(mutex.hpp)
#include <process/util/mutex_rw.hpp>

namespace fhatos {
  template<typename T, typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 250>
  class MutexDeque {
    enum ACCESS { READ, WRITE, LOCK };

  protected:
    Deque<T> deque_;
    Mutex<WAIT_TIME_MS> mutex_;

  private:
    template<typename A = void *>
    A lockUnlock(const bool withMutex, Supplier<A> function, ACCESS access = LOCK) {
      // remove withMutex and replace with ACCESS.OPEN
      if (withMutex)
        return access == READ ? mutex_.lockUnlock(function) : mutex_.lockUnlock(function);
      else {
        A temp = function();
        return temp;
      }
    }

  public:
    // MutexDeque(Mutex *mutex = new Mutex()) : _mutex(mutex) {}
    explicit MutexDeque(const char *label = "<anon>") : mutex_(label) {
    }

    bool exists(const Predicate<T> &predicate, const bool with_mutex = true) {
      return lockUnlock<bool>(with_mutex, [this, predicate]() {
        for (const T &t: this->deque_) {
          if (predicate(t))
            return true;
        }
        return false;
      }, READ);
    }

    Option<T> find(const Predicate<T> &predicate, const bool with_mutex = true) {
      return lockUnlock<Option<T>>(with_mutex, [this, predicate]() {
        for (const T &t: this->deque_) {
          if (predicate(t)) {
            return Option<T>(t);
          }
        }
        return Option<T>();
      }, READ);
    }

    List<T> find_all(const Predicate<T> &predicate, const bool with_mutex = true) {
      return lockUnlock<List<T>>(with_mutex, [this, predicate]() {
        List<T> list;
        for (const T &t: deque_) {
          if (predicate(t)) {
            list.push_back(t);
          }
        }
        return list;
      }, READ);
    }

    Option<T> pop_front(const bool with_mutex = true) {
      return lockUnlock<Option<T>>(with_mutex, [this]() {
        if (deque_.empty()) {
          return Option<T>();
        } else {
          const T t = this->deque_.front();
          deque_.pop_front();
          return Option<T>(t);
        }
      }, WRITE);
    }

    List_p<T> match(const Predicate<T> &predicate, const bool with_mutex = true) {
      auto results = share(List<T>());
      lockUnlock<void *>(with_mutex, [this, results, predicate]() {
        for (const T &t: deque_) {
          if (predicate(t))
            results->push_back(t);
        }
        return nullptr;
      }, READ);
      return results;
    }

    void forEach(const Consumer<T> &consumer, const bool with_mutex = true) {
      lockUnlock<void *>(with_mutex, [this, consumer]() {
        for (const T &t: deque_) {
          consumer(t);
        }
        return nullptr;
      }, WRITE);
    }

    Option<T> get(const int index, const bool with_mutex = true) {
      return lockUnlock<Option<T>>(with_mutex, [this, index]() {
        int counter = 0;
        for (const T &t: deque_) {
          if (counter++ == index) {
            return Option<T>(t);
            break;
          }
        }
        return Option<T>();
      }, READ);
    }

    List_p<T> remove_if(const Predicate<T> &predicate, const bool with_mutex = true) {
      auto *removed = new List<T>();
      lockUnlock<void *>(with_mutex, [this, predicate, removed] {
        deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                    [predicate, removed](T t) {
                                      const bool r = predicate(t);
                                      if (r)
                                        removed->push_back(t);
                                      return r;
                                    }),
                     deque_.end());
        return nullptr;
      }, WRITE);
      const List_p<T> removed_p = ptr<List<T>>(removed);
      return removed_p;
    }

    void remove(const T &to_remove, const bool with_mutex = true) {
      this->remove_if([this, to_remove](T t) { return t == to_remove; }, with_mutex);
    }

    Option<T> pop_back(const bool with_mutex = true) {
      return lockUnlock<Option<T>>(with_mutex, [this]() {
        if (deque_.empty())
          return Option<T>();
        const T t = deque_.back();
        deque_.pop_back();
        return Option<T>(t);
      }, WRITE);
    }

    bool push_front(const T t, const bool with_mutex = true) {
      return lockUnlock<bool>(with_mutex, [this, t]() {
        deque_.push_front(t);
        return true;
      }, WRITE);
    }

    bool push_back(const T t, const bool with_mutex = true) {
      return lockUnlock<bool>(with_mutex, [this, t]() {
        this->deque_.push_back(t);
        return true;
      }, WRITE);
    }

    SIZE_TYPE size(const bool with_mutex = true) {
      return lockUnlock<SIZE_TYPE>(with_mutex, [this]() { return this->deque_.size(); }, READ);
    }

    bool empty(const bool with_mutex = true) {
      return lockUnlock<bool>(with_mutex, [this]() { return this->deque_.empty(); }, READ);
    }

    string toString(const bool with_mutex = true) {
      return lockUnlock<string>(with_mutex, [this]() {
        string temp = "[";
        for (const auto &t: this->deque_) {
          if (t)
            temp = temp + t->toString() + ", ";
        }
        temp = temp.substr(0, temp.length() - 3) + "]";
        return temp;
      }, READ);
    }

    void clear(const bool with_mutex = true) {
      lockUnlock<void *>(with_mutex, [this]() {
        if (!this->deque_.empty())
          this->deque_.clear();
        return nullptr;
      }, WRITE);
    }
  };
} // namespace fhatos

#endif
