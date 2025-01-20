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

#include  "../fhatos.hpp"
#include <shared_mutex>

namespace fhatos {
  template<typename T, typename SIZE_TYPE = uint8_t>
  class MutexDeque {
    enum ACCESS { READ, WRITE, LOCK };

  protected:
    Deque<T> deque_;
    std::shared_mutex deque_mutex_;

  public:
    explicit MutexDeque(const char *label = "<anon>") {
    }

    void read_lock(const bool with_mutex) {
      if(with_mutex) {
        std::shared_lock<std::shared_mutex> lock(this->deque_mutex_);
        FEED_WATCDOG();
      }
    }

    void write_lock(const bool with_mutex) {
      if(with_mutex) {
        std::unique_lock<std::shared_mutex> lock(this->deque_mutex_);
        FEED_WATCDOG();
      }
    }

    bool exists(const Predicate<T> &predicate, const bool with_mutex = true) {
      read_lock(with_mutex);
      for(const T &t: this->deque_) {
        if(predicate(t))
          return true;
      }
      return false;
    }

    Option<T> find(const Predicate<T> &predicate, const bool with_mutex = true) {
      read_lock(with_mutex);
      for(const T &t: this->deque_) {
        if(predicate(t)) {
          return Option<T>(t);
        }
      }
      return Option<T>();
    }

    List<T> find_all(const Predicate<T> &predicate, const bool with_mutex = true) {
      read_lock(with_mutex);
      List<T> list;
      for(const T &t: deque_) {
        if(predicate(t)) {
          list.push_back(t);
        }
      }
      return list;
    }

    Option<T> pop_front(const bool with_mutex = true) {
      write_lock(with_mutex);
      if(deque_.empty()) {
        return Option<T>();
      } else {
        const T t = this->deque_.front();
        deque_.pop_front();
        return Option<T>(t);
      }
    }

    List_p<T> match(const Predicate<T> &predicate, const bool with_mutex = true) {
      read_lock(with_mutex);
      auto results = make_shared<List<T>>();
      for(const T &t: deque_) {
        if(predicate(t))
          results->push_back(t);
      }
      return results;
    }

    void forEach(const Consumer<T> &consumer, const bool with_mutex = true) {
      read_lock(with_mutex);
      for(const T &t: deque_) {
        consumer(t);
      }
    }

    Option<T> get(const int index, const bool with_mutex = true) {
      read_lock(with_mutex);
      int counter = 0;
      for(const T &t: deque_) {
        if(counter++ == index) {
          return Option<T>(t);
          break;
        }
      }
      return Option<T>();
    }

    typename std::deque<T>::iterator begin() {
      return this->deque_.begin();
    }

    typename std::deque<T>::iterator end() {
      return this->deque_.end();
    }


    void remove_if(const Predicate<T> &predicate, const bool with_mutex = true) {
      write_lock(with_mutex);
      deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                  [predicate](T t) {
                                    return predicate(t);
                                  }),
                   deque_.end());
    }

    List_p<T> remove_if_list(const Predicate<T> &predicate, const bool with_mutex = true) {
      write_lock(with_mutex);
      ptr<List<T>> removed = make_shared<List<T>>();
      deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                  [predicate, removed](T t) {
                                    const bool r = predicate(t);
                                    if(r)
                                      removed->push_back(t);
                                    return r;
                                  }),
                   deque_.end());
      return removed;
    }

    int remove_if_count(const Predicate<T> &predicate, const bool with_mutex = true) {
      write_lock(with_mutex);
      const auto count = make_shared<atomic_int>(0);
      deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                  [predicate, count](T t) {
                                    const bool r = predicate(t);
                                    if(r) count->fetch_add(1);
                                    return r;
                                  }),
                   deque_.end());
      const int c = count->load();
      return c;
    }

    void remove(const T &to_remove, const bool with_mutex = true) {
      this->remove_if([this, to_remove](T t) { return t == to_remove; }, with_mutex);
    }

    Option<T> pop_back(const bool with_mutex = true) {
      write_lock(with_mutex);
      if(deque_.empty())
        return Option<T>();
      const T t = deque_.back();
      deque_.pop_back();
      return Option<T>(t);
    }

    bool push_front(const T t, const bool with_mutex = true) {
      write_lock(with_mutex);
      deque_.push_front(t);
      return true;
    }

    bool push_back(const T t, const bool with_mutex = true) {
      write_lock(with_mutex);
      this->deque_.push_back(t);
      return true;
    }

    SIZE_TYPE size(const bool with_mutex = true) {
      read_lock(with_mutex);
      return this->deque_.size();
    }

    bool empty(const bool with_mutex = true) {
      read_lock(with_mutex);
      return this->deque_.empty();
    }

    string toString(const bool with_mutex = true) {
      read_lock(with_mutex);
      string temp = "[";
      for(const auto &t: this->deque_) {
        if(t)
          temp = temp + t->toString() + ", ";
      }
      temp = temp.substr(0, temp.length() - 3) + "]";
      return temp;
    }

    void clear(const bool with_mutex = true) {
      write_lock(with_mutex);
      if(!this->deque_.empty())
        this->deque_.clear();
    }
  };
} // namespace fhatos

#endif
