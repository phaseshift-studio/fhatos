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

#ifndef fhatos_fs_hpp
#define fhatos_fs_hpp

#include <fhatos.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <language/obj.hpp>

namespace fhatos {

  template<typename PROCESS = Coroutine>
  class FS : public PROCESS, public Structure {

  protected:
    MutexDeque<Mail_p> *OUTGOING = new MutexDeque<Mail_p>();
    List<Subscription_p> *SUBSCRIPTIONS = new List<Subscription_p>();
    MutexRW<> MUTEX_SUBSCRIPTIONS = MutexRW<>();

  public:
    FS(const ID &id, const Pattern &pattern) : PROCESS(id), Structure(pattern, SType::READWRITE) {

    }
  };

} // namespace fhatos

#endif
