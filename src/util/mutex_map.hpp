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
#ifndef fhatos_mutex_map_hpp
#define fhatos_mutex_map_hpp

#include "../model/fos/sys/scheduler/thread/mutex.hpp"
#include <shared_mutex>

namespace fhatos {
  template<typename KEY, typename VALUE,
           typename COMPARATOR = std::less<KEY>,
           typename ALLOCATOR = std::allocator<std::pair<const KEY, VALUE>>>
  class MutexMap {
  protected:
    std::map<KEY, VALUE, COMPARATOR, ALLOCATOR> map_;
    Mutex mutex_;

  public:
    explicit MutexMap() = default;

    std::map<KEY, VALUE> &get_base() const {
      return this->map_;
    }

    auto begin() const {
      return this->map_.begin();
    }

    auto end() const {
      return this->map_.end();
    }

    [[nodiscard]] VALUE at(const KEY &key) {
      auto lock = std::shared_lock<Mutex>(this->mutex_);
      return this->map_.at(key);
    }

    void erase(const KEY &key) {
      auto lock = std::lock_guard<Mutex>(this->mutex_);
      this->map_.erase(key);
    }


    [[nodiscard]] size_t count(const KEY &key) {
      auto lock = std::shared_lock<Mutex>(this->mutex_);
      return this->map_.count(key);
    }

    [[nodiscard]] bool exists(const KEY &key) {
      return this->count(key) > 0;
    }

    [[nodiscard]] bool empty() {
      auto lock = std::shared_lock<Mutex>(this->mutex_);
      return this->map_.empty();
    }

    [[nodiscard]] size_t size() {
      auto lock = std::shared_lock<Mutex>(this->mutex_);
      return this->map_.size();
    }

    void insert_or_assign(const KEY &key, const VALUE &&value) {
      auto lock = std::lock_guard<Mutex>(this->mutex_);
      this->map_.insert_or_assign(key, value);
    }

    std::pair<const KEY, VALUE> pop() {
      auto lock = std::lock_guard<Mutex>(this->mutex_);
      auto pair = std::pair<ID,Obj_p>(*this->map_.begin());
      this->map_.erase(pair.first);
      return pair;
    }

    void clear() {
      auto lock = std::lock_guard<Mutex>(this->mutex_);
      this->map_.clear();
    }
  };
}
#endif
