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
#ifndef fhatos_swarm_hpp
#define fhatos_swarm_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/pubsub.hpp>
#include <util/enums.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>
#include <process/actor/actor.hpp>

namespace fhatos {
  class Swarm;


  class Swarm : public IDed {
    friend class System;

  protected:
    ptr<List<Process_p>> actors_ = std::make_shared<List<Process_p>>();
    MutexRW<> mutex_ = MutexRW<>();

    explicit Swarm(const ID &id): IDed(id_p(id)) {
    }

  public:
    static ptr<Swarm> singleton() {
      static ptr<Swarm> swarm = ptr<Swarm>(new Swarm("/sys/swarm/"));
      return swarm;
    }

    template<typename ACTOR>
    ptr<ACTOR> deploy(const ptr<ACTOR> &actor) {
      router()->attach(actor);
      scheduler()->spawn(actor);
      this->actors_->push_back(actor);
      return actor;
    }
  };
} // namespace fhatos

#endif
