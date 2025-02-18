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
#include "../model/fos/sys/thread/fmutex.hpp"
#include <shared_mutex>

namespace fhatos {
  template<typename T, typename SIZE_TYPE = uint8_t>
  class MutexDeque {
  protected:
    Deque<T> deque_;
    fMutex deque_mutex_;

  public:
    explicit MutexDeque() = default;

    Deque<T>& get_base() {
      return this->deque_;
    }

    bool exists(const Predicate<T> &predicate) {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      for(const T &t: this->deque_) {
        if(predicate(t))
          return true;
      }
      return false;
    }

    Option<T> find(const Predicate<T> &predicate) {
      std::shared_lock<fMutex> lock(this->deque_mutex_);
      for(const T &t: this->deque_) {
        if(predicate(t)) {
          return Option<T>(t);
        }
      }
      return Option<T>();
    }

    std::vector<T> find_all(const Predicate<T> &predicate) {
      std::shared_lock<fMutex> lock(this->deque_mutex_);
      std::vector<T> list;
      for(const T &t: deque_) {
        if(predicate(t)) {
          list.push_back(t);
        }
      }
      return list;
    }

    Option<T> pop_front() {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      if(deque_.empty()) {
        return Option<T>();
      } else {
        const T t = this->deque_.front();
        deque_.pop_front();
        return Option<T>(t);
      }
    }

    std::vector<T> match(const Predicate<T> &predicate) {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      std::vector<T> results;
      for(const T &t: deque_) {
        if(predicate(t))
          results.push_back(t);
      }
      return results;
    }

    void forEach(const Consumer<T> &consumer) {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      for(const T &t: deque_) {
        consumer(t);
      }
    }

    Option<T> get(const int index) {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
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


    void remove_if(const Predicate<T> &predicate) {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                  [predicate](T t) {
                                    return predicate(t);
                                  }),
                   deque_.end());
    }

    std::vector<T> remove_if_list(const Predicate<T> &predicate) {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      std::vector<T> removed;
      deque_.erase(std::remove_if(deque_.begin(), deque_.end(),
                                  [predicate, removed](T t) {
                                    const bool r = predicate(t);
                                    if(r)
                                      removed.push_back(t);
                                    return r;
                                  }),
                   deque_.end());
      return removed;
    }

    int remove_if_count(const Predicate<T> &predicate) {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      const ptr<atomic_int> count = make_shared<atomic_int>(0);
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

    void remove(const T &to_remove) {
      this->remove_if([this, to_remove](T t) { return t == to_remove; });
    }

    Option<T> pop_back() {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      if(deque_.empty())
        return Option<T>();
      const T t = deque_.back();
      deque_.pop_back();
      return Option<T>(t);
    }

    bool push_front(const T t) {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      deque_.push_front(t);
      return true;
    }

    bool push_back(const T t) {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      this->deque_.push_back(t);
      return true;
    }

    SIZE_TYPE size() {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      return this->deque_.size();
    }

    bool empty() {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      return this->deque_.empty();
    }

    string toString() {
      auto lock = std::shared_lock<fMutex>(this->deque_mutex_);
      string temp = "[";
      for(const auto &t: this->deque_) {
        if(t)
          temp = temp + t->toString() + ", ";
      }
      temp = temp.substr(0, temp.length() - 3) + "]";
      return temp;
    }

    void clear() {
      auto lock = std::lock_guard<fMutex>(this->deque_mutex_);
      if(!this->deque_.empty())
        this->deque_.clear();
    }
  };
} // namespace fhatos

#endif
