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

#ifndef fhatos_distributed_memory_hpp
#define fhatos_distributed_memory_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_MQTT(mqtt.hpp)

namespace fhatos {

/**
 * @brief A distributed shared memory structure supporting multiple authorities via @code{//+/+}.
 */
  class DistributedMemory : public Actor<Fiber, Mqtt> {

  protected:
    explicit DistributedMemory(const ID &id = "/net/mem", const Pattern &pattern = "//+/+") :
            Actor<Fiber, Mqtt>(id, pattern) {}

  public:
    static ptr<DistributedMemory> create(const ID &id = "/net/mem", const Pattern &pattern = "//+/+") {
      ptr<DistributedMemory> memory_p = ptr<DistributedMemory>(new DistributedMemory(id, pattern));
      return memory_p;
    }
  };


} // namespace fhatos

#endif
