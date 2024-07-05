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
#ifndef fhatos_mailbox_hpp
#define fhatos_mailbox_hpp

#include <fhatos.hpp>

namespace fhatos {
  template<typename T>
  struct Mailbox {
  public:
    virtual ~Mailbox() {
      this->clear();
    }

    void clear() {
      while (this->pop().has_value()) {
      }
    }

    virtual bool push(const T message) { return true; }

    virtual uint16_t size() { return 0; }

    virtual Option<T> pop() { return {}; }
  };
} // namespace fhatos

#endif
