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
#include FOS_UTIL(mutex.hpp)

namespace fhatos {
  template<typename T, typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 250>
  class MutexDeque {
  protected:
    Deque<T> _deque;
    Mutex<WAIT_TIME_MS> _mutex;

  private:
    template<typename A = void *>
    A lockUnlock(const bool withMutex, Supplier<A> function) {
      if (withMutex)
        return _mutex.lockUnlock(function, WAIT_TIME_MS);
      else {
        A temp = function();
        return temp;
      }
    }

  public:
    // MutexDeque(Mutex *mutex = new Mutex()) : _mutex(mutex) {}
    explicit MutexDeque(const char *label = "<anon>") : _mutex(label) {}

    Option<T> find(Predicate<T> predicate, const bool withMutex = true) {
      return lockUnlock<Option<T>>(withMutex, [this, predicate]() {
        T *temp = nullptr;
        for (T t: _deque) {
          if (predicate(t)) {
            temp = &t;
            break;
          }
        }
        return temp ? Option<T>(*temp) : Option<T>();
      });
    }

    List<T> find_all(Predicate<T> predicate, const bool withMutex = true) {
      return lockUnlock<List<T>>(withMutex, [this, predicate]() {
        List<T> list;
        for (T t: _deque) {
          if (predicate(t)) {
            list.push_back(t);
          }
        }
        return list;
      });
    }

    Option<T> pop_front(const bool withMutex = true) {
      return lockUnlock<Option<T>>(withMutex, [this]() {
        if (_deque.empty()) {
          return Option<T>();
        } else {
          T t = _deque.front();
          _deque.pop_front();
          return Option<T>(t);
        }
      });
    }

    List_p<T> match(const Predicate<T> predicate, const bool withMutex = true) {
      auto results = share(List<T>());
      lockUnlock<void *>(withMutex, [this, results, predicate]() {
        for (const T &t: _deque) {
          if (predicate(t))
            results->push_back(t);
        }
        return nullptr;
      });
      return results;
    }

    void forEach(Consumer<T> consumer, const bool withMutex = true) {
      lockUnlock<void *>(withMutex, [this, consumer]() {
        for (const T &t: _deque) {
          consumer(t);
        }
        return nullptr;
      });
    }

    Option<T> get(int index, const bool withMutex = true) {
      return lockUnlock<Option<T>>(withMutex, [this, index]() {
        int counter = 0;
        for (const T &t: _deque) {
          if (counter++ == index) {
            return Option<T>(t);
            break;
          }
        }
        return Option<T>();
      });
    }

    List_p<T> remove_if(Predicate<T> predicate, const bool withMutex = true) {
      auto *removed = new List<T>();
      lockUnlock<void *>(withMutex, [this, predicate, removed] {
        _deque.erase(std::remove_if(_deque.begin(), _deque.end(),
                                    [predicate, removed](T t) {
                                      const bool r = predicate(t);
                                      if (r)
                                        removed->push_back(t);
                                      return r;
                                    }),
                     _deque.end());
        return nullptr;
      });
      const List_p<T> removed_p = ptr<List<T>>(removed);
      return removed_p;
    }

    void remove(const T &toRemove, const bool withMutex = true) {
      this->remove_if([this, toRemove](T t) { return t == toRemove; }, withMutex);
    }

    Option<T> pop_back(const bool withMutex = true) {
      return lockUnlock<Option<T>>(withMutex, [this]() {
        if (_deque.empty())
          return Option<T>();
        T t = _deque.back();
        _deque.pop_back();
        return Option<T>(t);
      });
    }

    bool push_front(const T t, const bool withMutex = true) {
      return lockUnlock<bool>(withMutex, [this, t]() {
        _deque.push_front(t);
        return true;
      });
    }

    bool push_back(const T t, const bool withMutex = true) {
      return lockUnlock<bool>(withMutex, [this, t]() {
        _deque.push_back(t);
        return true;
      });
    }

    SIZE_TYPE size(const bool withMutex = true) {
      return lockUnlock<SIZE_TYPE>(withMutex, [this]() { return _deque.size(); });
    }

    bool empty(const bool withMutex = true) {
      return lockUnlock<bool>(withMutex, [this]() { return _deque.empty(); });
    }

    string toString(const bool withMutex = true) {
      return lockUnlock<string>(withMutex, [this]() {
        string temp = "[";
        for (const auto &t: _deque) {
          if (t)
            temp = temp + t->toString() + ", ";
        }
        temp = temp.substr(0, temp.length() - 3) + "]";
        return temp;
      });
    }

    void clear(const bool withMutex = true) {
      lockUnlock<void *>(withMutex, [this]() {
        if (!_deque.empty())
          _deque.clear();
        return nullptr;
      });
    }
  };
} // namespace fhatos

#endif
