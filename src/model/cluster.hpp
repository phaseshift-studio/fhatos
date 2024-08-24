//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
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

#ifndef fhatos_cluster_hpp
#define fhatos_cluster_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_MQTT(mqtt.hpp)

namespace fhatos {

  class Cluster : public Actor<Fiber, Mqtt> {

  protected:
    explicit Cluster(const ID &id = "/io/cluster", const Pattern &pattern = "//+/#") : Actor<Fiber, Mqtt>(id,
                                                                                                          pattern) {}

  public:
    static ptr<Cluster> create(const ID &id = "/net/cluster", const Pattern &pattern = "//+/#") {
      ptr<Cluster> cluster_p = ptr<Cluster>(new Cluster(id, pattern));
      return cluster_p;
    }
  };


} // namespace fhatos

#endif
