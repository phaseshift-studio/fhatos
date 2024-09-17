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

#ifndef fhatos_shared_memory_hpp
#define fhatos_shared_memory_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <structure/stype/heap.hpp>

namespace fhatos {

  class SharedMemory : public Actor<Fiber, Heap> {

  protected:
    explicit SharedMemory(const ID &id = "/memory/shared", const Pattern &pattern = "+") :
            Actor(id, pattern) {}

  public:
    static ptr<SharedMemory> create(const ID &id = "/memory/shared", const Pattern &pattern = "+") {
      auto cluster_p = ptr<SharedMemory>(new SharedMemory(id, pattern));
      return cluster_p;
    }
  };

} // namespace fhatos

#endif
