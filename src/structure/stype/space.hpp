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

#pragma once
#ifndef fhatos_space_hpp
#define fhatos_space_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/rooter.hpp>
#include <structure/structure.hpp>

namespace fhatos {

  class Space : public Structure {

  protected:
    Map<ID, const Message_p> *DATA;
    MutexRW<> MUTEX_DATA = MutexRW<>();

  public:
    Space(const Pattern_p &range) : Structure(range, SType::READWRITE) {}

    void publish(const Message_p &message) const { Rooter::singleton()->publish(message); }

    void write(const Message_p &message) override {
      if (message->retain) {
        MUTEX_DATA.write<Obj>([this, message]() {
          Obj_p ret = Obj::to_noobj();
          if (DATA->count(message->target)) {
            ret = DATA->at(message->target)->payload;
            DATA->erase(message->target);
          }
          DATA->insert({message->target, message});
          return ret;
        });
      }
    }

    void read(const Subscription_p &subscription) override {
      MUTEX_DATA.read<void *>([this, subscription]() {
        for (const auto &[id, message]: *DATA) {
          if (id.matches(subscription->pattern))
            this->publish(message);
        }
        return nullptr;
      });
    }
  };
} // namespace fhatos

#endif
