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
#ifndef fhatos_global_hpp
#define fhatos_global_hpp

#include "../fhatos.hpp"
#include "../furi.hpp"
#include <shared_mutex>

namespace fhatos {
  class GLOBAL;
  using GLOBAL_p = ptr<GLOBAL>;

  class GLOBAL {
    std::shared_mutex map_mutex;
    const unique_ptr<Map<ID_p, Any, furi_p_less>> data_ = make_unique<Map<ID_p, Any, furi_p_less>>();

  public:
    static GLOBAL_p singleton() {
      static auto global = make_shared<GLOBAL>();
      return global;
    }

    template <typename T>
    T store(const ID_p &id, const T &thing) {
      auto lock = std::lock_guard(this->map_mutex);
      this->data_->insert_or_assign(id, thing);
      return thing;
    }

    bool exists(const ID_p &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      return this->data_->count(id) > 0;
    }

    template<typename T>
    T load(const ID_p &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      if(!this->data_->count(id))
        throw fError::create(id->toString(), "no global value associated with !b%s!!", id->toString().c_str());
      const Any thing = this->data_->at(id);
      const T t = std::any_cast<T>(thing);
      return t;
    }

    void remove(const ID_p &id) {
      auto lock = std::lock_guard(this->map_mutex);
      if(!this->data_->count(id))
        return;
      this->data_->erase(id);
    }
  };
}

#endif
