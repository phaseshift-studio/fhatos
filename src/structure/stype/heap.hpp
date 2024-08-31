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

//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef fhatos_heap_hpp
#define fhatos_heap_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/key_value.hpp>

namespace fhatos {

  class Heap : public KeyValue {

  protected:
    explicit Heap(const Pattern &pattern = "+") : KeyValue(pattern) {}

  public:
    static ptr<Heap> create(const Pattern &pattern) {
      ptr<Heap> heap_p = ptr<Heap>(new Heap(pattern));
      return heap_p;
    }

    // TODO: subscriptions bound to keys (garbage collection driven by subscription)
  };


} // namespace fhatos

#endif
