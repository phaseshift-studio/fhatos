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

#include "../model/fos/sys/thread/fmutex.hpp"
#include <shared_mutex>

namespace fhatos {
  template<typename K, typename V>
  class MutexMap {
  protected:
    std::map<const K, V> map_;
    fMutex mutex_;

  public:
    explicit MutexMap() = default;

    std::map<K, V> get_base() const {
      return this->map_;
    }

    auto begin() const {
      return this->map_.begin();
    }

    auto end() const {
      return this->map_.end();
    }

    V at(const K &key) {
      auto lock = std::shared_lock<fMutex>(this->mutex_);
      return this->map_.at(key);
    }

    void erase(const K &key) {
      auto lock = std::lock_guard<fMutex>(this->mutex_);
      this->map_.erase(key);
    }


    size_t count(const K &key) {
      auto lock = std::shared_lock<fMutex>(this->mutex_);
      return this->map_.count(key);
    }

    [[nodiscard]] bool empty() {
      auto lock = std::shared_lock<fMutex>(this->mutex_);
      return this->map_.empty();
    }

    void insert_or_assign(const K &key, V &&value) {
      auto lock = std::lock_guard<fMutex>(this->mutex_);
      this->map_.insert_or_assign(key, value);
    }

    void clear() {
      auto lock = std::lock_guard<fMutex>(this->mutex_);
      this->map_.clear();
    }
  };
}
#endif
